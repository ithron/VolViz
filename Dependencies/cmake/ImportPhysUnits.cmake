cmake_minimum_required(VERSION 3.2 FATAL_ERROR)

add_library(VolViz::PhysUnits INTERFACE IMPORTED)
if(XCODE)
  set_property(TARGET VolViz::PhysUnits PROPERTY INTERFACE_COMPILE_OPTIONS
    -isystem${DEPENDENCIES_DIR}/PhysUnits
  )
else()
  set_property(TARGET VolViz::PhysUnits PROPERTY INTERFACE_INCLUDE_DIRECTORIES
    ${DEPENDENCIES_DIR}/PhysUnits
  )
  set_property(TARGET VolViz::PhysUnits PROPERTY INTERFACE_SYSTEM_INCLUDE_DIRECTORIES
    ${DEPENDENCIES_DIR}/PhysUnits
  )
endif()

