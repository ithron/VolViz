cmake_minimum_required(VERSION 3.2 FATAL_ERROR)

if (NOT TARGET Eigen)

  set(INSTALL_EIGEN ON)

  add_library(Eigen INTERFACE)
  if(XCODE)
    set_property(TARGET Eigen PROPERTY INTERFACE_COMPILE_OPTIONS
      $<BUILD_INTERFACE:-isystem ${DEPENDENCIES_DIR}/Eigen-3.2.8>
    )
  else()
    set_property(TARGET Eigen PROPERTY INTERFACE_INCLUDE_DIRECTORIES
      $<BUILD_INTERFACE:${DEPENDENCIES_DIR}/Eigen-3.2.8>
    )
    set_property(TARGET Eigen PROPERTY INTERFACE_SYSTEM_INCLUDE_DIRECTORIES
      $<BUILD_INTERFACE:${DEPENDENCIES_DIR}/Eigen-3.2.8>
    )
  endif()

endif()

install(TARGETS Eigen EXPORT VolVizExport)

if(INSTALL_EIGEN)
  install(DIRECTORY
    ${DEPENDENCIES_DIR}/Eigen-3.2.8/Eigen
    DESTINATION include/VolViz/src
  )
endif()
