cmake_minimum_required(VERSION 3.2 FATAL_ERROR)

if (NOT HAS_EIGEN)
  set(HAS_EIGEN YES)
  add_library(Eigen INTERFACE)
  if(XCODE)
    set_property(TARGET Eigen PROPERTY INTERFACE_COMPILE_OPTIONS
      -isystem$<BUILD_INTERFACE:${DEPENDENCIES_DIR}/Eigen-3.2.8>$<INSTALL_INTERFACE:include/VolViz/src/Eigen>
    )
  else()
    set_property(TARGET Eigen PROPERTY INTERFACE_INCLUDE_DIRECTORIES
      $<BUILD_INTERFACE:${DEPENDENCIES_DIR}/Eigen-3.2.8>
      $<INSTALL_INTERFACE:include/VolViz/src/Eigen>
    )
    set_property(TARGET Eigen PROPERTY INTERFACE_SYSTEM_INCLUDE_DIRECTORIES
      $<BUILD_INTERFACE:${DEPENDENCIES_DIR}/Eigen-3.2.8>
      $<INSTALL_INTERFACE:include/VolViz/src/Eigen>
    )
  endif()

  install(TARGETS Eigen EXPORT VolVizExport)
  install(DIRECTORY
    ${DEPENDENCIES_DIR}/Eigen-3.2.8/Eigen
    DESTINATION include/VolViz/src
  )
endif()

# cmake_minimum_required(VERSION 3.2 FATAL_ERROR)
#
# add_library(Eigen INTERFACE)
# target_include_directories(Eigen INTERFACE
#   $<BUILD_INTERFACE:${DEPENDENCIES_DIR}/Eigen/include/Eigen-3.2.8>
#   $<INSTALL_INTERFACE:include/Eigen>
# )
#
# install(TARGETS Eigen EXPORT eigenExport)
# install(EXPORT eigenExport NAMESPACE Upstream::
#   DESTINATION lib/cmake/Eigen
# )
# install(DIRECTORY
#   ${DEPENDENCIES_DIR}/Eigen-3.2.8/Eigen
#   DESTINATION include/Eigen
# )
#
