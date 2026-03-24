cmake_minimum_required(VERSION 3.20)

include(FetchContent)

FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)

FetchContent_MakeAvailable(googletest)

include(GoogleTest)

add_executable(run_tests
    test_expression.cpp
    test_conversion.cpp
    test_equations.cpp
    test_errors.cpp
    test_divisors.cpp
    test_unit_conversions.cpp
)

target_link_libraries(run_tests
  PRIVATE
    gtest_main
    calculator_core
)

gtest_discover_tests(run_tests
  DISCOVERY_MODE POST_BUILD
)
