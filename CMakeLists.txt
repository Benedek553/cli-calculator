cmake_minimum_required(VERSION 3.10)

project(cli-calculator VERSION 2.2.0 LANGUAGES CXX)

# Recommended: out-of-source builds
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_TESTING "Enable testing" ON)

if(BUILD_TESTING)
  include(CTest)
endif()

# Enable good warnings for GCC/Clang
if(CMAKE_CXX_COMPILER_ID MATCHES "(GNU|Clang)")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

add_subdirectory(src)

if(BUILD_TESTING)
  add_subdirectory(tests)
  enable_testing()

  set(TEST_WORKDIR_EMPTY_VARS "${CMAKE_BINARY_DIR}/test_workdirs/empty_vars")
  file(MAKE_DIRECTORY ${TEST_WORKDIR_EMPTY_VARS})
  file(REMOVE ${TEST_WORKDIR_EMPTY_VARS}/vars.toml)

  add_test(
    NAME calculator_eval_basic
    COMMAND ${CMAKE_COMMAND}
      -Dcmd=$<TARGET_FILE:calculator>
      "-Dargs=--no-color;--eval;1+2*3"
      -Dexpected_exit_code=0
      "-Dpattern=Result:[ ]7"
      -P ${CMAKE_SOURCE_DIR}/cmake/run_with_expectations.cmake)

  add_test(
    NAME calculator_eval_missing_expr
    COMMAND ${CMAKE_COMMAND}
      -Dcmd=$<TARGET_FILE:calculator>
      "-Dargs=--no-color;--eval"
      -Dexpected_exit_code=1
      "-Dpattern=missing expression"
      -P ${CMAKE_SOURCE_DIR}/cmake/run_with_expectations.cmake)

  add_test(
    NAME calculator_square_root_value
    COMMAND ${CMAKE_COMMAND}
      -Dcmd=$<TARGET_FILE:calculator>
      "-Dargs=--no-color;--square-root;9"
      -Dexpected_exit_code=0
      "-Dpattern=Result:[ ]3"
      -P ${CMAKE_SOURCE_DIR}/cmake/run_with_expectations.cmake)

  add_test(
    NAME calculator_square_root_negative
    COMMAND ${CMAKE_COMMAND}
      -Dcmd=$<TARGET_FILE:calculator>
      "-Dargs=--no-color;--square-root;-9"
      -Dexpected_exit_code=1
      "-Dpattern=square root undefined for negative values"
      -P ${CMAKE_SOURCE_DIR}/cmake/run_with_expectations.cmake)

  add_test(
    NAME calculator_version
    COMMAND ${CMAKE_COMMAND}
      -Dcmd=$<TARGET_FILE:calculator>
      "-Dargs=--no-color;--version"
      -Dexpected_exit_code=0
      "-Dpattern=CLI Calculator version[ ]${PROJECT_VERSION}"
      -P ${CMAKE_SOURCE_DIR}/cmake/run_with_expectations.cmake)

  add_test(
    NAME calculator_unknown_flag
    COMMAND ${CMAKE_COMMAND}
      -Dcmd=$<TARGET_FILE:calculator>
      "-Dargs=--no-color;--bogus"
      -Dexpected_exit_code=1
      "-Dpattern=unknown argument: --bogus"
      -P ${CMAKE_SOURCE_DIR}/cmake/run_with_expectations.cmake)

  add_test(
    NAME calculator_output_without_action
    COMMAND ${CMAKE_COMMAND}
      -Dcmd=$<TARGET_FILE:calculator>
      "-Dargs=--output;json"
      -Dexpected_exit_code=1
      "-Dpattern=structured output requires a CLI action flag"
      -P ${CMAKE_SOURCE_DIR}/cmake/run_with_expectations.cmake)

  add_test(
    NAME calculator_variables_empty
    COMMAND ${CMAKE_COMMAND}
      -Dcmd=$<TARGET_FILE:calculator>
      "-Dargs=--no-color;--variables"
      -Dexpected_exit_code=0
      "-Dpattern=No variables stored"
      -Dworking_directory=${TEST_WORKDIR_EMPTY_VARS}
      -P ${CMAKE_SOURCE_DIR}/cmake/run_with_expectations.cmake)
endif()

install(TARGETS divisors calculator
        RUNTIME DESTINATION bin)

if(TARGET calculator_gui)
  install(TARGETS calculator_gui
          BUNDLE DESTINATION .)
endif()


