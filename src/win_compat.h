/* src/win_compat.h */
#pragma once
#ifdef _MSC_VER

/* Quiet MSVC's "secure" CRT complaints for legacy, POSIX-y code paths */
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS 1
#endif

/* GCC/Clang niceties become no-ops on MSVC */
#ifndef __attribute__
#define __attribute__(x)
#endif
#ifndef __builtin_expect
#define __builtin_expect(x, y) (x)
#endif

/* ssize_t for MSVC */
#ifndef ssize_t
  #include <BaseTsd.h>
  typedef SSIZE_T ssize_t;
#endif

/* POSIX-ish I/O shims */
#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <direct.h>

/* Open flags that sometimes aren't defined on MSVC */
#ifndef O_BINARY
  #define O_BINARY _O_BINARY
#endif
#ifndef O_RDONLY
  #define O_RDONLY _O_RDONLY
#endif
#ifndef O_WRONLY
  #define O_WRONLY _O_WRONLY
#endif
#ifndef O_RDWR
  #define O_RDWR   _O_RDWR
#endif
#ifndef O_CREAT
  #define O_CREAT  _O_CREAT
#endif
#ifndef O_TRUNC
  #define O_TRUNC  _O_TRUNC
#endif

/* Map common POSIX names to MSVC CRT */
#ifndef open
  #define open   _open
#endif
#ifndef read
  #define read   _read
#endif
#ifndef write
  #define write  _write
#endif
#ifndef close
  #define close  _close
#endif
#ifndef lseek
  #define lseek  _lseek
#endif
#ifndef unlink
  #define unlink _unlink
#endif
#ifndef fileno
  #define fileno _fileno
#endif
#ifndef isatty
  #define isatty _isatty
#endif
#ifndef dup
  #define dup    _dup
#endif

/* mkdir(path, mode) → _mkdir(path) */
#ifndef mkdir
  #define mkdir(p, m) _mkdir(p)
#endif

/* ftruncate shim (only if nobody else provided one, e.g. a local unistd shim) */
#if !defined(ftruncate) && !defined(LIBGPOD_HAVE_FTRUNCATE_SHIM)
  #define LIBGPOD_HAVE_FTRUNCATE_SHIM 1
  static __inline int ftruncate(int fd, long long size) {
    return _chsize_s(fd, (__int64)size) == 0 ? 0 : -1;
  }
#endif

/* timezone: map POSIX 'timezone' var (and '__timezone' if used) to MSVC '_timezone' */
#include <time.h>
#ifndef timezone
  #ifdef _timezone
    #define timezone _timezone
  #else
    extern long _timezone;
    #define timezone _timezone
  #endif
#endif
#ifndef __timezone
  #define __timezone timezone
#endif

#endif /* _MSC_VER */

