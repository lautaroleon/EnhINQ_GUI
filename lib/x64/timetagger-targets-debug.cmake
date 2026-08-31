#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "TimeTagger::TimeTagger" for configuration "Debug"
set_property(TARGET TimeTagger::TimeTagger APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(TimeTagger::TimeTagger PROPERTIES
  IMPORTED_IMPLIB_DEBUG "${_IMPORT_PREFIX}/TimeTaggerD.lib"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/TimeTaggerD.dll"
  )

list(APPEND _cmake_import_check_targets TimeTagger::TimeTagger )
list(APPEND _cmake_import_check_files_for_TimeTagger::TimeTagger "${_IMPORT_PREFIX}/TimeTaggerD.lib" "${_IMPORT_PREFIX}/TimeTaggerD.dll" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
