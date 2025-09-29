#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "unofficial::libimobiledevice-glue::libimobiledevice-glue" for configuration "Release"
set_property(TARGET unofficial::libimobiledevice-glue::libimobiledevice-glue APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(unofficial::libimobiledevice-glue::libimobiledevice-glue PROPERTIES
  IMPORTED_IMPLIB_RELEASE "${_IMPORT_PREFIX}/lib/imobiledevice-glue-1.0.lib"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/bin/imobiledevice-glue-1.0.dll"
  )

list(APPEND _cmake_import_check_targets unofficial::libimobiledevice-glue::libimobiledevice-glue )
list(APPEND _cmake_import_check_files_for_unofficial::libimobiledevice-glue::libimobiledevice-glue "${_IMPORT_PREFIX}/lib/imobiledevice-glue-1.0.lib" "${_IMPORT_PREFIX}/bin/imobiledevice-glue-1.0.dll" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
