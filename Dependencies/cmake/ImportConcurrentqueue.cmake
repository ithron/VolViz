cmake_minimum_required(VERSION 3.2 FATAL_ERROR)

if(NOT ConcurrentQueue)

  add_library(ConcurrentQueue INTERFACE)
  if("${CMAKE_GENERATOR}" STREQUAL "Xcode")
    set_property(TARGET ConcurrentQueue PROPERTY INTERFACE_COMPILE_OPTIONS
      $<BUILD_INTERFACE:-isystem ${DEPENDENCIES_DIR}/concurrentqueue
    )
  else()
    set_property(TARGET ConcurrentQueue PROPERTY INTERFACE_INCLUDE_DIRECTORIES
      $<BUILD_INTERFACE:${DEPENDENCIES_DIR}/concurrentqueue>
    )
    set_property(TARGET ConcurrentQueue PROPERTY INTERFACE_SYSTEM_INCLUDE_DIRECTORIES
      $<BUILD_INTERFACE:${DEPENDENCIES_DIR}/concurrentqueue>
    )
  endif()

endif()

install(TARGETS ConcurrentQueue EXPORT VolVizExport)
