#----------------------------------------------------------------
# Generated CMake target import file for configuration "RelWithDebInfo".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "VolViz::VolViz" for configuration "RelWithDebInfo"
set_property(TARGET VolViz::VolViz APPEND PROPERTY IMPORTED_CONFIGURATIONS RELWITHDEBINFO)
set_target_properties(VolViz::VolViz PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELWITHDEBINFO "CXX"
  IMPORTED_LOCATION_RELWITHDEBINFO "${_IMPORT_PREFIX}/lib/libVolViz.a"
  )

list(APPEND _IMPORT_CHECK_TARGETS VolViz::VolViz )
list(APPEND _IMPORT_CHECK_FILES_FOR_VolViz::VolViz "${_IMPORT_PREFIX}/lib/libVolViz.a" )

# Import target "VolViz::VolVisualizer" for configuration "RelWithDebInfo"
set_property(TARGET VolViz::VolVisualizer APPEND PROPERTY IMPORTED_CONFIGURATIONS RELWITHDEBINFO)
set_target_properties(VolViz::VolVisualizer PROPERTIES
  IMPORTED_LOCATION_RELWITHDEBINFO "${_IMPORT_PREFIX}/bin/VolVisualizer"
  )

list(APPEND _IMPORT_CHECK_TARGETS VolViz::VolVisualizer )
list(APPEND _IMPORT_CHECK_FILES_FOR_VolViz::VolVisualizer "${_IMPORT_PREFIX}/bin/VolVisualizer" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
