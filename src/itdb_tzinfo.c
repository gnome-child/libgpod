/*
|  Copyright (C) 2002-2007 Jorg Schuler <jcsjcs at users sourceforge net>
|  Copyright (C) 2009 Christophe Fergeau <cfergeau at mandriva com>
|  Part of the gtkpod project.
| 
|  URL: http://www.gtkpod.org/
|  URL: http://gtkpod.sourceforge.net/
|
|  The code contained in this file is free software; you can redistribute
|  it and/or modify it under the terms of the GNU Lesser General Public
|  License as published by the Free Software Foundation; either version
|  2.1 of the License, or (at your option) any later version.
|
|  This file is distributed in the hope that it will be useful,
|  but WITHOUT ANY WARRANTY; without even the implied warranty of
|  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
|  Lesser General Public License for more details.
|
|  You should have received a copy of the GNU Lesser General Public
|  License along with this code; if not, write to the Free Software
|  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
|  USA
|
|  iTunes and iPod are trademarks of Apple
|
|  This product is not supported/written/published by Apple!
|
|  $Id$
*/
#include <config.h>

#include "itdb.h"
#include "itdb_device.h"
#include "itdb_private.h"
#include "itdb_tzinfo_data.h"

#include <stdio.h>
#include <string.h>
#include <time.h>

#include <glib/gstdio.h>
#include <time.h>

/* ---- Portable timezone accessor (seconds west of UTC) ---- */
#if defined(_MSC_VER)
  /* MSVC has _get_timezone(long*) */
  static inline long gpod_tz_seconds(void) {
    long z = 0;
    _get_timezone(&z);
    return z;
  }
#elif defined(__CYGWIN__)
  /* Cygwin exposes _timezone as an import */
  extern __IMPORT long _timezone;
  static inline long gpod_tz_seconds(void) {
    return _timezone;
  }
#else
  /* POSIX: 'timezone' is a long, usually from <time.h> */
  extern long timezone;
  static inline long gpod_tz_seconds(void) {
    return timezone;
  }
#endif

static gboolean parse_tzdata (const char *tzname, time_t start, time_t end,
			      int *offset, gboolean *has_dst, int *dst_offset);

/* Mac epoch is 1st January 1904 00:00 in local time */
G_GNUC_INTERNAL time_t device_time_mac_to_time_t (Itdb_Device *device, guint64 mactime)
{
    g_return_val_if_fail (device, 0);
    if (mactime != 0)  return (time_t)(mactime - 2082844800 - device->timezone_shift);
    else               return (time_t)mactime;
}

G_GNUC_INTERNAL guint64 device_time_time_t_to_mac (Itdb_Device *device, time_t timet)
{
    g_return_val_if_fail (device, 0);
    if (timet != 0)
	return ((guint64)timet) + 2082844800 + device->timezone_shift;
    else return 0;
}

static char *
get_preferences_path (const Itdb_Device *device)
{

    const gchar *p_preferences[] = {"Preferences", NULL};
    char *dev_path;
    char *prefs_filename;

    if (device->mountpoint == NULL) {
        return NULL;
    }

    dev_path = itdb_get_device_dir (device->mountpoint);

    if (dev_path == NULL) {
        return NULL;
    }

    prefs_filename = itdb_resolve_path (dev_path, p_preferences);
    g_free (dev_path);

    return prefs_filename;
}

static gboolean itdb_device_read_raw_timezone (const char *prefs_path,
                                               glong offset,
                                               gint16 *out_raw_tz)
{
    FILE *f;
    int result;

    if (out_raw_tz == NULL) {
        return FALSE;
    }

    f = fopen (prefs_path, "r");
    if (f == NULL) {
        return FALSE;
    }

    result = fseek (f, offset, SEEK_SET);
    if (result != 0) {
        fclose (f);
        return FALSE;
    }

    result = fread (out_raw_tz, sizeof (*out_raw_tz), 1, f);
    if (result != 1) {
        fclose (f);
        return FALSE;
    }

    fclose (f);

    *out_raw_tz = GINT16_FROM_LE (*out_raw_tz);

    return TRUE;
}

static gboolean raw_timezone_to_utc_shift_4g (gint16 raw_timezone,
                                              gint *utc_shift)
{
    const int GMT_OFFSET = 0x19;

    if (utc_shift == NULL) {
        return FALSE;
    }

    if ((raw_timezone < 0) || (raw_timezone > (2*12) << 1)) {
        /* invalid timezone */
        return FALSE;
    }

    raw_timezone -= GMT_OFFSET;

    *utc_shift = (raw_timezone >> 1) * 3600;
    if (raw_timezone & 1) {
        /* Adjust for DST */
        *utc_shift += 3600;
    }

    return TRUE;
}

static gboolean raw_timezone_to_utc_shift_5g (gint16 raw_timezone,
                                              gint *utc_shift)
{
    const int TZ_SHIFT = 8;

    if (utc_shift == NULL) {
        return FALSE;
    }
    /* The iPod stores the timezone information as a number of minutes
     * from Tokyo timezone which increases when going eastward (ie
     * going from Tokyo to LA and then to Europe).
     * The calculation below shifts the origin so that 0 corresponds
     * to UTC-12 and the max is 24*60 and corresponds to UTC+12
     * Finally, we subtract 12*60 to that value to get a signed number
     * giving the timezone relative to UTC.
     */
    *utc_shift = raw_timezone*60 - TZ_SHIFT*3600;

    return TRUE;
}

static gboolean raw_timezone_to_utc_shift_6g (gint16 city_id,
                                              gint *utc_shift)
{
    const ItdbTimezone *it;

    for (it = timezones; it->city_name != NULL; it++) {
	if (it->city_index == city_id) {
	    int offset;
	    gboolean unused1;
	    int unused2;
	    gboolean got_tzinfo;

	    got_tzinfo = parse_tzdata (it->tz_name, time (NULL), time (NULL),
				       &offset, &unused1, &unused2);
	    if (!got_tzinfo) {
		return FALSE;
	    }
	    *utc_shift = offset*60;

	    return TRUE;
	}
    }

    /* unknown city ID */
    return FALSE;
}

static gint get_local_timezone (void)
{
    /* gpod_tz_seconds() returns seconds west of UTC; negate to get east-of-UTC */
    return (gint)(-gpod_tz_seconds());
}

/* This function reads the timezone information from the iPod and sets it in
 * the Itdb_Device structure. If an error occurs, the function returns silently
 * and the timezone shift is set to 0
 */
G_GNUC_INTERNAL void itdb_device_set_timezone_info (Itdb_Device *device)
{
    gint16 raw_timezone;
    gint utc_shift_seconds = 0;
    gboolean result;
    struct stat stat_buf;
    int status;
    char *prefs_path;
    guint offset;
    gboolean (*raw_timezone_converter) (gint16 raw_timezone, gint *utc_shift);

    device->timezone_shift = get_local_timezone ();

    prefs_path = get_preferences_path (device);

    if (!prefs_path) {
	return;
    }

    status = g_stat (prefs_path, &stat_buf);
    if (status != 0) {
	g_free (prefs_path);
	return;
    }
    switch (stat_buf.st_size) {
	case 2892: /* seen on iPod 4g */
	    offset = 0xb10;
	    raw_timezone_converter = raw_timezone_to_utc_shift_4g;
	    break;
	case 2924: /* seen on iPod video */
	    offset = 0xb22;
	    raw_timezone_converter = raw_timezone_to_utc_shift_5g;
	    break;
	case 2952: /* seen on nano 3g and iPod classic */
	case 2956: /* seen on iPod classic */
	case 2960: /* seen on nano 4g */
	    offset = 0xb70;
	    raw_timezone_converter = raw_timezone_to_utc_shift_6g;
	    break;
	default:
	    /* We don't know how to get the timezone of this ipod model,
	     * assume the computer timezone and the ipod timezone match
	     */
	    g_free (prefs_path);
	    return; 
    }

    result = itdb_device_read_raw_timezone (prefs_path, offset, 
					    &raw_timezone);
    g_free (prefs_path);
    if (!result) {
	return;
    }
    result = raw_timezone_converter (raw_timezone, &utc_shift_seconds);
    if (!result) {
	return;
    }

    if ((utc_shift_seconds < -12*3600) || (utc_shift_seconds > 12 * 3600)) {
         return;
    }
    
    device->timezone_shift = utc_shift_seconds;
}


/*
 * This function replaces the old parse_tzdata commented out below.
 *
 * The old one assumed a particular file format and crashed on Windows.
 *
 * We avoid that by using GTimeZone instead.
 */
static gboolean
parse_tzdata (const char *tzname, time_t start, time_t end,
              int *offset, gboolean *has_dst, int *dst_offset)
{
    (void)end;

    GTimeZone *tz = g_time_zone_new (tzname);
    if (!tz)
        return FALSE;

    gint64 when = (gint64) start;
    gint interval = g_time_zone_find_interval (tz, G_TIME_TYPE_UNIVERSAL, when);
    if (interval == -1) {
        g_time_zone_unref (tz);
        return FALSE;
    }

    gint seconds = g_time_zone_get_offset (tz, interval);
    *offset = seconds / 60;

    if (has_dst)     *has_dst = FALSE;
    if (dst_offset)  *dst_offset = *offset;

    g_time_zone_unref (tz);
    return TRUE;
}

// /*
//  * The following function was copied from libgweather/gweather-timezone.c
//  *
//  * Copyright 2008, Red Hat, Inc.
//  *
//  * This library is free software; you can redistribute it and/or
//  * modify it under the terms of the GNU Lesser General Public License
//  * as published by the Free Software Foundation; either version 2.1 of
//  * the License, or (at your option) any later version.
//  *
//  * libgweather uses it for a different purpose than libgpod's, namely
//  * finding if a given world location uses DST (not 'now' but at some point
//  * during the year) and getting the offset. 
//  * libgpod only needs to know the offset from UTC, and if start == end when
//  * calling this function, this is exactly what we get!
//  */
// #define ZONEINFO_DIR "/usr/share/zoneinfo"

// #define TZ_MAGIC "TZif"
// #define TZ_HEADER_SIZE 44
// #define TZ_TIMECNT_OFFSET 32
// #define TZ_TRANSITIONS_OFFSET 44

// #define TZ_TTINFO_SIZE 6
// #define TZ_TTINFO_GMTOFF_OFFSET 0
// #define TZ_TTINFO_ISDST_OFFSET 4

// static gboolean
// parse_tzdata (const char *_tzname, time_t start, time_t end,
// 	      int *offset, gboolean *has_dst, int *dst_offset)
// {
//     char *filename, *contents;
//     gsize length;
//     int timecnt, transitions_size, ttinfo_map_size;
//     int initial_transition = -1, second_transition = -1;
//     gint32 *transitions;
//     char *ttinfo_map, *ttinfos;
//     gint32 initial_offset, second_offset;
//     char initial_isdst, second_isdst;
//     int i;

//     filename = g_build_filename (ZONEINFO_DIR, _tzname, NULL);
//     if (!g_file_get_contents (filename, &contents, &length, NULL)) {
// 	g_free (filename);
// 	return FALSE;
//     }
//     g_free (filename);

//     if (length < TZ_HEADER_SIZE ||
// 	strncmp (contents, TZ_MAGIC, strlen (TZ_MAGIC)) != 0) {
// 	g_free (contents);
// 	return FALSE;
//     }

//     timecnt = GUINT32_FROM_BE (*(guint32 *)(contents + TZ_TIMECNT_OFFSET));
//     transitions = (void *)(contents + TZ_TRANSITIONS_OFFSET);
//     transitions_size = timecnt * sizeof (*transitions);
//     ttinfo_map = (void *)(contents + TZ_TRANSITIONS_OFFSET + transitions_size);
//     ttinfo_map_size = timecnt;
//     ttinfos = (void *)(ttinfo_map + ttinfo_map_size);

//     /* @transitions is an array of @timecnt time_t values. We need to
//      * find the transition into the current offset, which is the last
//      * transition before @start. If the following transition is before
//      * @end, then note that one too, since it presumably means we're
//      * doing DST.
//      */
//     for (i = 1; i < timecnt && initial_transition == -1; i++) {
// 	if (GINT32_FROM_BE (transitions[i]) > start) {
// 	    initial_transition = ttinfo_map[i - 1];
// 	    if (GINT32_FROM_BE (transitions[i]) < end)
// 		second_transition = ttinfo_map[i];
// 	}
//     }
//     if (initial_transition == -1) {
// 	if (timecnt)
// 	    initial_transition = ttinfo_map[timecnt - 1];
// 	else
// 	    initial_transition = 0;
//     }

//     /* Copy the data out of the corresponding ttinfo structs */
//     initial_offset = *(gint32 *)(ttinfos +
// 				 initial_transition * TZ_TTINFO_SIZE +
// 				 TZ_TTINFO_GMTOFF_OFFSET);
//     initial_offset = GINT32_FROM_BE (initial_offset);
//     initial_isdst = *(ttinfos +
// 		      initial_transition * TZ_TTINFO_SIZE +
// 		      TZ_TTINFO_ISDST_OFFSET);

//     if (second_transition != -1) {
// 	second_offset = *(gint32 *)(ttinfos +
// 				    second_transition * TZ_TTINFO_SIZE +
// 				    TZ_TTINFO_GMTOFF_OFFSET);
// 	second_offset = GINT32_FROM_BE (second_offset);
// 	second_isdst = *(ttinfos +
// 			 second_transition * TZ_TTINFO_SIZE +
// 			 TZ_TTINFO_ISDST_OFFSET);

// 	*has_dst = (initial_isdst != second_isdst);
//     } else
// 	*has_dst = FALSE;

//     if (!*has_dst)
// 	*offset = initial_offset / 60;
//     else {
// 	if (initial_isdst) {
// 	    *offset = second_offset / 60;
// 	    *dst_offset = initial_offset / 60;
// 	} else {
// 	    *offset = initial_offset / 60;
// 	    *dst_offset = second_offset / 60;
// 	}
//     }

//     g_free (contents);
//     return TRUE;
// }
