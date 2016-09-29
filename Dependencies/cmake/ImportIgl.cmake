cmake_minimum_required(VERSION 3.2 FATAL_ERROR)

add_library(IGL INTERFACE IMPORTED)
if(XCODE)
  set_property(TARGET IGL PROPERTY INTERFACE_COMPILE_OPTIONS
    -isystem${DEPENDENCIES_DIR}/libigl/include
  )
else()
  set_property(TARGET IGL PROPERTY INTERFACE_INCLUDE_DIRECTORIES
    ${DEPENDENCIES_DIR}/libigl/include
  )
  set_property(TARGET IGL PROPERTY INTERFACE_SYSTEM_INCLUDE_DIRECTORIES
    ${DEPENDENCIES_DIR}/libigl/include
  )
endif()

