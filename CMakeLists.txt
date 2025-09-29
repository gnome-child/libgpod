cmake_minimum_required(VERSION 3.16)
project(libgpod C)

if (MSVC)
  # Treat as modern C, good warnings
  add_compile_options(/W4 /permissive-)

  # Force-include our Windows shim before any headers are parsed
  add_compile_options(/FI"${CMAKE_SOURCE_DIR}/src/win_compat.h")

  # Ensure our local shim 'src/unistd.h' is found before system includes
  include_directories(BEFORE "${CMAKE_SOURCE_DIR}/src")
endif()

include(CheckIncludeFiles)
include(FindPkgConfig)
include(GNUInstallDirs)

# --- Toolchain / vcpkg hints (harmless elsewhere) ---
# Helps CMake/pkconfig find vcpkg-installed libs on Windows
if(DEFINED ENV{VCPKG_ROOT} AND DEFINED VCPKG_TARGET_TRIPLET)
  list(APPEND CMAKE_PREFIX_PATH "$ENV{VCPKG_ROOT}/installed/${VCPKG_TARGET_TRIPLET}")
endif()

# --- Warnings & language flags ---
set(COMPILE_OPTIONS "")

if(MSVC)
  # Force-include our Windows shims with an absolute path so MSVC always finds it.
  add_compile_options(/FI"${CMAKE_CURRENT_SOURCE_DIR}/src/win_compat.h")

  add_compile_definitions(
    _CRT_SECURE_NO_WARNINGS
    NOMINMAX
    WIN32_LEAN_AND_MEAN
  )

  list(APPEND COMPILE_OPTIONS
    /W4 /permissive-
    /wd4100  # unreferenced formal parameter (common in GObject callbacks)
    /wd4068  # unknown pragma (for #pragma GCC diagnostic)
    /wd4244  # narrowing conversions (lots of guint/char)
    /wd4996  # POSIX names like unlink (shimmed in win_compat.h)
    $<$<COMPILE_LANGUAGE:C>:/std:c11>
  )
else()
  list(APPEND COMPILE_OPTIONS
    -std=c99
    -Wall -Wextra -Wpedantic
    -Wunused -Wshadow -Wuninitialized
    -Wredundant-decls -Winit-self
    -Wmissing-include-dirs -Wmissing-declarations
    -Wstrict-overflow=2 -Wunused-parameter
    -Wformat=2 -Wdisabled-optimization
  )
  if(CMAKE_C_COMPILER_ID MATCHES "GNU")
    list(APPEND COMPILE_OPTIONS -Wno-cast-function-type)
  endif()
endif()

option(BUILD_WERROR "Build with -Werror" OFF)
if(BUILD_WERROR)
  if(MSVC)
    list(APPEND COMPILE_OPTIONS /WX)
  else()
    list(APPEND COMPILE_OPTIONS -Werror)
  endif()
endif()

add_compile_options(${COMPILE_OPTIONS})

# --- Sources ---
set(SOURCES
  src/itdb_thumb.c
  src/itdb_track.c
  src/itdb_zlib.c
  src/pixmaps.c
  src/itdb_itunesdb.c
  src/db-parse-context.c
  src/db-artwork-parser.c
  src/db-artwork-writer.c
  src/itdb_artwork.c
  src/itdb_iphone.c
  src/itdb_playlist.c
  src/rijndael.c
  src/ithumb-writer.c
  src/itdb_tzinfo.c
  src/itdb_photoalbum.c
  src/itdb_hash58.c
  src/db-image-parser.c
  src/itdb_hash72.c
  src/itdb_sqlite.c
  src/itdb_device.c
  src/itdb_sysinfo_extended_parser.c
  src/itdb_chapterdata.c
  src/db-artwork-debug.c
  src/itdb_hashAB.c
  src/itdb_plist.c
)

# --- Dependencies via pkg-config ---
find_package(PkgConfig REQUIRED)

pkg_check_modules(GLIB REQUIRED glib-2.0)
pkg_check_modules(GMODULE REQUIRED gmodule-2.0)
pkg_check_modules(GOBJECT REQUIRED gobject-2.0)
pkg_check_modules(GDK_PIXBUF gdk-pixbuf-2.0)
# libplist can be called libplist-2.0 or libplist depending on distro/vcpkg
pkg_search_module(LIBPLIST REQUIRED libplist-2.0 libplist)
pkg_check_modules(LIBIMOBILEDEVICE libimobiledevice-1.0)
pkg_check_modules(SQLITE3 REQUIRED sqlite3)
pkg_check_modules(ZLIB REQUIRED zlib)

# POSIX headers: only probe on non-Windows
if(NOT WIN32)
  check_include_files(unistd.h HAVE_UNISTD_H)
else()
  set(HAVE_UNISTD_H 0)
endif()

option(ENABLE_GDKPIXBUF "Build with gdk-pixbuf support" ${GDK_PIXBUF_FOUND})
option(ENABLE_LIBIMOBILEDEVICE "Build with libimobiledevice support" ${LIBIMOBILEDEVICE_FOUND})

set(HAVE_GDKPIXBUF ${ENABLE_GDKPIXBUF})
set(HAVE_LIBIMOBILEDEVICE ${ENABLE_LIBIMOBILEDEVICE})

set(LIBGPOD_BLOB_DIR "${CMAKE_INSTALL_PREFIX}/libgpod")
set(VERSION 0.8.0)

if(HAVE_GDKPIXBUF)
  set(GDKPIXBUF_REQ "gdk-pixbuf-2.0")
endif()

if(HAVE_LIBIMOBILEDEVICE)
  set(LIBIMOBILEDEVICE_REQ "libimobiledevice-1.0")
endif()

if(GLIB_VERSION GREATER_EQUAL 2.68)
  set(HAVE_G_MEMDUP2 ON)
endif()

configure_file(config.h.in config.h)
configure_file(libgpod-1.0.pc.in libgpod-1.0.pc @ONLY)

# Quoted properly as a compile definition
add_compile_definitions(GETTEXT_PACKAGE="libgpod")

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
  add_compile_definitions(HAVE_STRUCT_TM_TM_GMTOFF)
endif()
# add_compile_definitions(HAVE_LOCALTIME_R)

# MSVC needs explicit exports to produce an import lib.
if (MSVC)
  set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

add_library(gpod SHARED ${SOURCES})

target_include_directories(gpod SYSTEM PRIVATE
  ${GLIB_INCLUDE_DIRS}
  ${GMODULE_INCLUDE_DIRS}
  ${GOBJECT_INCLUDE_DIRS}
  ${LIBPLIST_INCLUDE_DIRS}
  ${SQLITE3_INCLUDE_DIRS}
  ${ZLIB_INCLUDE_DIRS}
)

target_include_directories(gpod PRIVATE
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_directories(gpod PRIVATE
  ${GLIB_LIBRARY_DIRS}
  ${GMODULE_LIBRARY_DIRS}
  ${GOBJECT_LIBRARY_DIRS}
  ${LIBPLIST_LIBRARY_DIRS}
  ${SQLITE3_LIBRARY_DIRS}
  ${ZLIB_LIBRARY_DIRS}
)

target_link_libraries(gpod PRIVATE
  ${GLIB_LIBRARIES}
  ${GMODULE_LIBRARIES}
  ${GOBJECT_LIBRARIES}
  ${LIBPLIST_LIBRARIES}
  ${SQLITE3_LIBRARIES}
  ${ZLIB_LIBRARIES}
)

if(ENABLE_GDKPIXBUF)
  target_include_directories(gpod SYSTEM PRIVATE ${GDK_PIXBUF_INCLUDE_DIRS})
  target_link_directories(gpod PRIVATE ${GDK_PIXBUF_LIBRARY_DIRS})
  target_link_libraries(gpod PRIVATE ${GDK_PIXBUF_LIBRARIES})
endif()

if(ENABLE_LIBIMOBILEDEVICE)
  target_include_directories(gpod SYSTEM PRIVATE ${LIBIMOBILEDEVICE_INCLUDE_DIRS})
  target_link_directories(gpod PRIVATE ${LIBIMOBILEDEVICE_LIBRARY_DIRS})
  target_link_libraries(gpod PRIVATE ${LIBIMOBILEDEVICE_LIBRARIES})
endif()

# Use GNUInstallDirs (portable on Windows, too)
install(TARGETS gpod
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(
  DIRECTORY src/
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/gpod-1.0/gpod
  FILES_MATCHING PATTERN itdb.h
)

install(
  FILES ${CMAKE_CURRENT_BINARY_DIR}/libgpod-1.0.pc
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
)

