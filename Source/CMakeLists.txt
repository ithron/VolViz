cmake_minimum_required(VERSION 3.2 FATAL_ERROR)

option(BUILD_APP "Build mesh viewer app?" ON)

if (BUILD_APP)
  set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${DEPENDENCIES_DIR}/cmake)
  include(ImportIgl)
endif()

set(DEFAULT_COMPILE_FEATURES
  cxx_alias_templates
  cxx_alignas
  cxx_auto_type
  cxx_binary_literals
  cxx_constexpr
  cxx_contextual_conversions
  cxx_decltype_incomplete_return_types
  cxx_decltype
  cxx_decltype_auto
  cxx_default_function_template_args
  cxx_defaulted_functions
  cxx_defaulted_move_initializers
  cxx_delegating_constructors
  cxx_deleted_functions
  cxx_explicit_conversions
  cxx_generalized_initializers
  cxx_generic_lambdas
  cxx_inheriting_constructors
  cxx_inline_namespaces
  cxx_lambdas
  cxx_lambda_init_captures
  cxx_long_long_type
  cxx_noexcept
  cxx_nullptr
  cxx_override
  cxx_range_for
  cxx_raw_string_literals
  cxx_reference_qualified_functions
  cxx_return_type_deduction
  cxx_right_angle_brackets
  cxx_rvalue_references
  cxx_sizeof_member
  cxx_static_assert
  cxx_strong_enums
  cxx_trailing_return_types
  cxx_uniform_initialization
  cxx_variadic_templates
  cxx_template_template_parameters
)

set(PUBLIC_HEADERS
  Visualizer.h
)


if (("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang") OR
  ("${CMAKE_CXX_COMPILER_ID}" MATCHES "AppleClang"))
  set(DEFAULT_COMPILER_OPTIONS
    -Werror
    -Weverything
#disable warnings
    -Qunused-arguments
    -Wno-unknown-warning-option
    -Wno-c++98-compat
    -Wno-c++98-compat-bind-to-temporary-copy
    -Wno-c++98-compat-pedantic
    -Wno-global-constructors
    -Wno-reserved-id-macro
    -Wno-documentation
    -Wno-documentation-unknown-command
    -Wno-old-style-cast
    -Wno-deprecated
    -Wno-missing-noreturn
    -Wno-used-but-marked-unused
    -Wno-padded
    -Wno-cast-align
    -Wno-double-promotion
    -Wno-sign-compare
  )
endif()
if (${CMAKE_COMPILER_IS_GNUCXX})
  set(DEFAULT_COMPILER_OPTIONS
    -Werror
    -Wall -Wextra -pedantic
#disable warnings
    -Wno-unknown-pragmas
    -Wno-comment
  )
endif()

add_library(VolViz
  GLFW.cpp
  ShaderProgram.cpp
  Shaders.cpp
  Visualizer.cpp
)
target_link_libraries(VolViz
  PUBLIC
    Eigen
    GSL
    simpleunit
  PRIVATE
    glfw
    ${OPENGL_LIBRARIES}
    ${MKL_TARGET}
)
target_include_directories(VolViz PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
  $<INSTALL_INTERFACE:include/VolViz>
)
target_compile_features(VolViz PUBLIC ${DEFAULT_COMPILE_FEATURES})
target_compile_options(VolViz PRIVATE ${DEFAULT_COMPILER_OPTIONS})

set(INSTALL_TARGETS VolViz)

add_library(VolViz::VolViz ALIAS VolViz)

if (BUILD_APP)
  add_executable(VolVisualizer VolVisualizer.cpp)
  target_link_libraries(VolVisualizer PRIVATE
    VolViz::VolViz
    IGL
  )
  target_compile_features(VolVisualizer PRIVATE ${DEFAULT_COMPILE_FEATURES})
  target_compile_options(VolVisualizer PRIVATE ${DEFAULT_COMPILER_OPTIONS})

  set(INSTALL_TARGETS ${INSTALL_TARGETS} VolVisualizer)
endif()

if (BUILD_TESTING)
#add test targets here
endif()

if (BUILD_TESTING)
#find_package(GTest REQUIRED)
  add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../Dependencies/googletest gtest)
endif()


###############################
#Install and packaging

install(
  TARGETS
    ${INSTALL_TARGETS}
  EXPORT
    VolVizExport
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
  INCLUDES DESTINATION include/VolViz
)
install(EXPORT VolVizExport NAMESPACE VolViz::
  DESTINATION lib/cmake/VolViz
)
install(FILES ${PUBLIC_HEADERS}
  DESTINATION include/VolViz
)

export(EXPORT VolVizExport NAMESPACE VolViz::)
include(CMakePackageConfigHelpers)
configure_package_config_file(VolVizConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/VolVizConfig.cmake
  INSTALL_DESTINATION lib/cmake/VolViz
)
configure_package_config_file(
  ${DEPENDENCIES_DIR}/cmake/FindMKL.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/FindMKL.cmake
  INSTALL_DESTINATION lib/cmake/VolViz
)
configure_package_config_file(
  ${DEPENDENCIES_DIR}/cmake/ImportEigen.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/ImportEigen.cmake
  INSTALL_DESTINATION lib/cmake/VolViz
)
configure_package_config_file(
  ${DEPENDENCIES_DIR}/cmake/ImportIgl.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/ImportIgl.cmake
  INSTALL_DESTINATION lib/cmake/VolViz
)
configure_package_config_file(
  ${DEPENDENCIES_DIR}/cmake/ImportMKL.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/ImportMKL.cmake
  INSTALL_DESTINATION lib/cmake/VolViz
)
install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/VolVizConfig.cmake
  DESTINATION lib/cmake/VolViz
)
