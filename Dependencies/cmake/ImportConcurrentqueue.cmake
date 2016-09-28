cmake_minimum_required(VERSION 3.2 FATAL_ERROR)

add_library(ConcurrentQueue INTERFACE IMPORTED)
set_property(TARGET ConcurrentQueue PROPERTY INTERFACE_INCLUDE_DIRECTORIES
  ${DEPENDENCIES_DIR}/concurrentqueue
)

