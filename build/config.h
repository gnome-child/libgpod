#ifndef CONFIG_H
#define CONFIG_H

/* Optional feature macros: always 0 or 1, filled in by xmake */

#define HAVE_GDKPIXBUF        1
#define HAVE_LIBIMOBILEDEVICE 1
#define HAVE_UNISTD_H         1
#define HAVE_G_MEMDUP2        1

/* Blob directory from build config */

#define LIBGPOD_BLOB_DIR "/libgpod"

#endif  // CONFIG_H
