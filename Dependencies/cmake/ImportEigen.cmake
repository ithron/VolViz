cmake_minimum_required(VERSION 3.2 FATAL_ERROR)

add_library(Eigen INTERFACE IMPORTED)
set_property(TARGET Eigen PROPERTY INTERFACE_INCLUDE_DIRECTORIES
  ${DEPENDENCIES_DIR}/Eigen-3.2.8
)

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
