#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "TimeTagger::TimeTagger" for configuration "Release"
set_property(TARGET TimeTagger::TimeTagger APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(TimeTagger::TimeTagger PROPERTIES
  IMPORTED_IMPLIB_RELEASE "${_IMPORT_PREFIX}/TimeTagger.lib"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/TimeTagger.dll"
  )

list(APPEND _cmake_import_check_targets TimeTagger::TimeTagger )
list(APPEND _cmake_import_check_files_for_TimeTagger::TimeTagger "${_IMPORT_PREFIX}/TimeTagger.lib" "${_IMPORT_PREFIX}/TimeTagger.dll" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
