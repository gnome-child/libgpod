#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "unofficial::libusbmuxd::libusbmuxd" for configuration "Release"
set_property(TARGET unofficial::libusbmuxd::libusbmuxd APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(unofficial::libusbmuxd::libusbmuxd PROPERTIES
  IMPORTED_IMPLIB_RELEASE "${_IMPORT_PREFIX}/lib/usbmuxd-2.0.lib"
  IMPORTED_LINK_DEPENDENT_LIBRARIES_RELEASE "unofficial::libplist::libplist;unofficial::libimobiledevice-glue::libimobiledevice-glue"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/bin/usbmuxd-2.0.dll"
  )

list(APPEND _cmake_import_check_targets unofficial::libusbmuxd::libusbmuxd )
list(APPEND _cmake_import_check_files_for_unofficial::libusbmuxd::libusbmuxd "${_IMPORT_PREFIX}/lib/usbmuxd-2.0.lib" "${_IMPORT_PREFIX}/bin/usbmuxd-2.0.dll" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
