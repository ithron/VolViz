cmake_minimum_required(VERSION 3.2 FATAL_ERROR)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${DEPENDENCIES_DIR}/cmake)
include(ImportIgl)

find_package(Threads REQUIRED)

set(DEFAULT_COMPILE_FEATURES
  cxx_alias_templates
  cxx_alignas
  cxx_auto_type
  cxx_binary_literals
  cxx_constexpr
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

add_executable(VolVisualizer VolVisualizer.cpp)
target_link_libraries(VolVisualizer PRIVATE
  VolViz::VolViz
  IGL
  ${CMAKE_THREAD_LIBS_INIT}
)
target_compile_features(VolVisualizer PRIVATE ${DEFAULT_COMPILE_FEATURES})
target_compile_options(VolVisualizer PRIVATE ${DEFAULT_COMPILER_OPTIONS})
if (${CMAKE_COMPILER_IS_GNUCXX})
  if (${CMAKE_C_COMPILER_VERSION} VERSION_GREATER "5" AND
      ${CMAKE_C_COMPILER_VERSION} VERSION_LESS "6")
    target_compile_options(VolVisualizer PRIVATE ${DEFAULT_COMPILER_OPTIONS} -w)
    message(WARNING "Disable -Werror for GCC-5 since libigl does not build otherwise")
  endif()
endif()


###############################
#Install and packaging

install(
  TARGETS
    VolVisualizer
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
  INCLUDES DESTINATION include/VolViz
)
