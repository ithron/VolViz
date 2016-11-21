cmake_minimum_required(VERSION 3.2 FATAL_ERROR)

if (NOT TARGET Eigen)

  add_library(Eigen INTERFACE)
  if(XCODE)
    set_property(TARGET Eigen PROPERTY INTERFACE_COMPILE_OPTIONS
      $<BUILD_INTERFACE:-isystem${DEPENDENCIES_DIR}/eigen>
    )
  else()
    set_property(TARGET Eigen PROPERTY INTERFACE_INCLUDE_DIRECTORIES
      $<BUILD_INTERFACE:${DEPENDENCIES_DIR}/eigen>
    )
    set_property(TARGET Eigen PROPERTY INTERFACE_SYSTEM_INCLUDE_DIRECTORIES
      $<BUILD_INTERFACE:${DEPENDENCIES_DIR}/eigen>
    )
  endif()
  target_link_libraries(Eigen INTERFACE ${MKL_LIBRARIES})

  install(TARGETS Eigen EXPORT VolVizExport)

  install(DIRECTORY
    ${DEPENDENCIES_DIR}/eigen/Eigen
    DESTINATION include/VolViz/src
  )
endif()

