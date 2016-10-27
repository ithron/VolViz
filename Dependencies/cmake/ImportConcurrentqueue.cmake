cmake_minimum_required(VERSION 3.2 FATAL_ERROR)

if(NOT HAS_ConcurrentQueue)
  set(HAS_ConcurrentQueue YES)

  add_library(ConcurrentQueue INTERFACE)
  if("${CMAKE_GENERATOR}" STREQUAL "Xcode")
    set_property(TARGET ConcurrentQueue PROPERTY INTERFACE_COMPILE_OPTIONS
      -isystem$<BUILD_INTERFACE:-isystem ${DEPENDENCIES_DIR}/concurrentqueue>
    )
  else()
    set_property(TARGET ConcurrentQueue PROPERTY INTERFACE_INCLUDE_DIRECTORIES
      $<BUILD_INTERFACE:${DEPENDENCIES_DIR}/concurrentqueue>
    )
    set_property(TARGET ConcurrentQueue PROPERTY INTERFACE_SYSTEM_INCLUDE_DIRECTORIES
      $<BUILD_INTERFACE:${DEPENDENCIES_DIR}/concurrentqueue>
    )
  endif()

  install(TARGETS ConcurrentQueue EXPORT VolVizExport)
endif()

