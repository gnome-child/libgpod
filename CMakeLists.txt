cmake_minimum_required(VERSION 3.16)

project(libgpod
  VERSION 0.8.0
  LANGUAGES C
)

include(GNUInstallDirs)
include(CheckIncludeFiles)
find_package(PkgConfig REQUIRED)

# --- Make pkg-config painless on Windows when using vcpkg ---
if (WIN32 AND DEFINED ENV{VCPKG_ROOT} AND DEFINED VCPKG_TARGET_TRIPLET)
  set(PKG_CONFIG_EXECUTABLE
      "$ENV{VCPKG_ROOT}/installed/${VCPKG_TARGET_TRIPLET}/tools/pkgconf/pkgconf"
      CACHE FILEPATH "pkg-config for the active vcpkg triplet")
  list(APPEND CMAKE_PREFIX_PATH
      "$ENV{VCPKG_ROOT}/installed/${VCPKG_TARGET_TRIPLET}")
endif()

# --- Options (keep optional deps OFF by default, except gdk-pixbuf) ---
option(ENABLE_GDKPIXBUF        "Build with gdk-pixbuf support" ON)
option(ENABLE_LIBIMOBILEDEVICE "Build with libimobiledevice support" OFF)
option(BUILD_WERROR            "Treat warnings as errors" OFF)

# --- Core deps (pkg-config imported targets) ---
pkg_check_modules(GLIB     REQUIRED IMPORTED_TARGET glib-2.0)
pkg_check_modules(GMODULE  REQUIRED IMPORTED_TARGET gmodule-2.0)
pkg_check_modules(GOBJECT  REQUIRED IMPORTED_TARGET gobject-2.0)
pkg_check_modules(SQLITE3  REQUIRED IMPORTED_TARGET sqlite3)
pkg_check_modules(ZLIB     REQUIRED IMPORTED_TARGET zlib)
pkg_search_module(LIBPLIST REQUIRED IMPORTED_TARGET libplist-2.0 libplist)

# Optional deps – probe only if requested
if(ENABLE_GDKPIXBUF)
  pkg_check_modules(GDKPIXBUF IMPORTED_TARGET gdk-pixbuf-2.0)
endif()

if(ENABLE_LIBIMOBILEDEVICE)
  pkg_check_modules(LIBIMOBILEDEVICE IMPORTED_TARGET libimobiledevice-1.0)
endif()

# --- Small platform probes / config.h ---
if(NOT WIN32)
  check_include_files(unistd.h HAVE_UNISTD_H)
else()
  set(HAVE_UNISTD_H 0)
endif()

# GLib >= 2.68 has g_memdup2
if(GLIB_VERSION VERSION_GREATER_EQUAL "2.68")
  set(HAVE_G_MEMDUP2 ON)
endif()

set(LIBGPOD_VERSION "${PROJECT_VERSION}")
set(LIBGPOD_BLOB_DIR "${CMAKE_INSTALL_PREFIX}/libgpod")

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/config.h.in"
               "${CMAKE_CURRENT_BINARY_DIR}/config.h" @ONLY)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/libgpod-1.0.pc.in"
               "${CMAKE_CURRENT_BINARY_DIR}/libgpod-1.0.pc" @ONLY)

# --- Sources & library ---
set(LIBGPOD_SOURCES
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

add_library(gpod SHARED ${LIBGPOD_SOURCES})

# C standard & warnings
target_compile_features(gpod PRIVATE c_std_11)
target_compile_options(gpod PRIVATE
  $<$<C_COMPILER_ID:MSVC>:/W4 /permissive- /wd4100 /wd4068 /wd4244 /wd4267 /wd4996>
  $<$<NOT:$<C_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic -Wformat=2 -Wshadow -Wredundant-decls -Wunused -Wuninitialized>
  $<$<AND:$<NOT:$<C_COMPILER_ID:MSVC>>,$<C_COMPILER_ID:GNU>>:-Wno-cast-function-type>
)
if(BUILD_WERROR)
  target_compile_options(gpod PRIVATE
    $<$<C_COMPILER_ID:MSVC>:/WX>
    $<$<NOT:$<C_COMPILER_ID:MSVC>>:-Werror>
  )
endif()

# Definitions
target_compile_definitions(gpod PRIVATE
  GETTEXT_PACKAGE="libgpod"
  $<$<PLATFORM_ID:Windows>:NOMINMAX WIN32_LEAN_AND_MEAN _CRT_SECURE_NO_WARNINGS>
  $<$<STREQUAL:${CMAKE_SYSTEM_NAME},Linux>:HAVE_STRUCT_TM_TM_GMTOFF>
)

# Windows niceties
if(MSVC)
  set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
  # If you have a Windows compatibility header, force-include it:
  target_compile_options(gpod PRIVATE "/FI${CMAKE_CURRENT_SOURCE_DIR}/src/win_compat.h")
  target_include_directories(gpod PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
endif()

# Includes (public + config.h)
target_include_directories(gpod
  PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${CMAKE_CURRENT_BINARY_DIR}"          # config.h
  PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/include"  # public headers
)

# Link deps (guard optionals by FOUND)
target_link_libraries(gpod PRIVATE
  PkgConfig::GLIB
  PkgConfig::GMODULE
  PkgConfig::GOBJECT
  PkgConfig::SQLITE3
  PkgConfig::ZLIB
  PkgConfig::LIBPLIST
  $<$<BOOL:${GDKPIXBUF_FOUND}>:PkgConfig::GDKPIXBUF>
  $<$<BOOL:${LIBIMOBILEDEVICE_FOUND}>:PkgConfig::LIBIMOBILEDEVICE>
)

set_target_properties(gpod PROPERTIES
  OUTPUT_NAME "gpod"
  VERSION   "${PROJECT_VERSION}"
  SOVERSION "${PROJECT_VERSION_MAJOR}"
)

# --- Install ---
install(TARGETS gpod
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(
  DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/gpod-1.0"
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
  FILES "${CMAKE_CURRENT_BINARY_DIR}/libgpod-1.0.pc"
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
)

# ---- Tests ----
# TODO
