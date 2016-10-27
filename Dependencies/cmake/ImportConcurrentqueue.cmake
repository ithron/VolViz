cmake_minimum_required(VERSION 3.2 FATAL_ERROR)

if(NOT ConcurrentQueue_INCLUDE_DIR)

  set(ConcurrentQueue_INCLUDE_DIR
    $<BUILD_INTERFACE:${DEPENDENCIES_DIR}/concurrentqueue>
  )
  set(ConcurrentQueue_INCLUDE_DIR_OPT
    $<BUILD_INTERFACE:-isystem ${DEPENDENCIES_DIR}/concurrentqueue
  )
endif()

add_library(ConcurrentQueue INTERFACE)
if("${CMAKE_GENERATOR}" STREQUAL "Xcode")
  set_property(TARGET ConcurrentQueue PROPERTY INTERFACE_COMPILE_OPTIONS
    ${ConcurrentQueue_INCLUDE_DIR_OPT}
  )
else()
  set_property(TARGET ConcurrentQueue PROPERTY INTERFACE_INCLUDE_DIRECTORIES
    ${ConcurrentQueue_INCLUDE_DIR}
  )
  set_property(TARGET ConcurrentQueue PROPERTY INTERFACE_SYSTEM_INCLUDE_DIRECTORIES
    ${ConcurrentQueue_INCLUDE_DIR}
  )
endif()

install(TARGETS ConcurrentQueue EXPORT VolVizExport)
