#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "unofficial::libplist::libplist" for configuration "Debug"
set_property(TARGET unofficial::libplist::libplist APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(unofficial::libplist::libplist PROPERTIES
  IMPORTED_IMPLIB_DEBUG "${_IMPORT_PREFIX}/debug/lib/plist-2.0.lib"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/debug/bin/plist-2.0.dll"
  )

list(APPEND _cmake_import_check_targets unofficial::libplist::libplist )
list(APPEND _cmake_import_check_files_for_unofficial::libplist::libplist "${_IMPORT_PREFIX}/debug/lib/plist-2.0.lib" "${_IMPORT_PREFIX}/debug/bin/plist-2.0.dll" )

# Import target "unofficial::libplist::libplist++" for configuration "Debug"
set_property(TARGET unofficial::libplist::libplist++ APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(unofficial::libplist::libplist++ PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "C;CXX"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/debug/lib/plist++-2.0.lib"
  )

list(APPEND _cmake_import_check_targets unofficial::libplist::libplist++ )
list(APPEND _cmake_import_check_files_for_unofficial::libplist::libplist++ "${_IMPORT_PREFIX}/debug/lib/plist++-2.0.lib" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
