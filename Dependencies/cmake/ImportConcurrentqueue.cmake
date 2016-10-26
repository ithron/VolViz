cmake_minimum_required(VERSION 3.2 FATAL_ERROR)

add_library(VolViz::ConcurrentQueue INTERFACE IMPORTED)
if("${CMAKE_GENERATOR}" STREQUAL "Xcode")
  set_property(TARGET VolViz::ConcurrentQueue PROPERTY INTERFACE_COMPILE_OPTIONS
    -isystem${DEPENDENCIES_DIR}/concurrentqueue
  )
else()
  set_property(TARGET VolViz::ConcurrentQueue PROPERTY INTERFACE_INCLUDE_DIRECTORIES
    ${DEPENDENCIES_DIR}/concurrentqueue
  )
  set_property(TARGET VolViz::ConcurrentQueue PROPERTY INTERFACE_SYSTEM_INCLUDE_DIRECTORIES
    ${DEPENDENCIES_DIR}/concurrentqueue
  )
endif()

