cmake_minimum_required(VERSION 3.2 FATAL_ERROR)

if (NOT TARGET PhysUnits)

  add_library(PhysUnits INTERFACE)
  if(XCODE)
    set_property(TARGET PhysUnits PROPERTY INTERFACE_COMPILE_OPTIONS
      -isystem$<BUILD_INTERFACE:${DEPENDENCIES_DIR}/PhysUnits>$<INSTALL_INTERFACE:include/VolViz/src/PhysUnits>
    )
  else()
    set_property(TARGET PhysUnits PROPERTY INTERFACE_INCLUDE_DIRECTORIES
      $<BUILD_INTERFACE:${DEPENDENCIES_DIR}/PhysUnits>
    )
    set_property(TARGET PhysUnits PROPERTY INTERFACE_SYSTEM_INCLUDE_DIRECTORIES
      $<BUILD_INTERFACE:${DEPENDENCIES_DIR}/PhysUnits>
    )
  endif()

  install(TARGETS PhysUnits EXPORT VolVizExport)
  install(DIRECTORY
    ${DEPENDENCIES_DIR}/PhysUnits/phys
    DESTINATION include/VolViz/src
  )

endif()
