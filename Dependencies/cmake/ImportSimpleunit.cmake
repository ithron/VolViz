cmake_minimum_required(VERSION 3.2 FATAL_ERROR)

add_library(simpleunit INTERFACE IMPORTED)
set_property(TARGET simpleunit PROPERTY INTERFACE_INCLUDE_DIRECTORIES
  ${DEPENDENCIES_DIR}/simpleunit
)
