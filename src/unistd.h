#pragma once
#ifdef _MSC_VER
  #include <io.h>
  #include <fcntl.h>
  #include <direct.h>
  #include <process.h>
  #include <sys/types.h>
  #include <sys/stat.h>

  /* open/flags */
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
  #ifndef O_BINARY
  #define O_BINARY _O_BINARY
  #endif

  /* POSIX-ish names */
  #define open    _open
  #define read    _read
  #define write   _write
  #define close   _close
  #define lseek   _lseek
  #define access  _access
  #define unlink  _unlink
  #define dup2    _dup2
  #define fileno  _fileno
  #define getpid  _getpid
  #define isatty  _isatty

  #if defined(_MSC_VER) && !defined(LIBGPOD_HAVE_FTRUNCATE_SHIM)
    #include <io.h>
    static inline int ftruncate(int fd, __int64 size) {
      return _chsize_s(fd, size) == 0 ? 0 : -1;
    }
#endif
#endif

