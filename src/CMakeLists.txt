cmake_minimum_required(VERSION 3.10)

find_package(ZLIB REQUIRED)
find_package(Boost REQUIRED)

set(CORE_SOURCES
    core/divisors_lib.cpp
    core/prime_factors.cpp
    core/equations.cpp
    core/expression.cpp
    core/variables.cpp
    core/input.cpp
    core/math_utils.cpp
    core/numeral_conversion.cpp
    core/matrix.cpp
    core/statistics.cpp
    core/graph_png.cpp
    core/unit_conversion.cpp
    core/parse_utils.cpp
)

set(APP_SOURCES
    main.cpp
    app/calculator_app.cpp
    app/cli_batch.cpp
    app/cli_commands.cpp
    app/cli_parser.cpp
    app/cli_repl.cpp
    app/cli_numeric.cpp
    app/cli_output.cpp
    app/menu_handlers.cpp
)

set(TOOLS_SOURCES
    tools/divisors.cpp
)

set(COMMON_INCLUDES
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/core
    ${CMAKE_CURRENT_SOURCE_DIR}/app
    ${CMAKE_CURRENT_SOURCE_DIR}/tools
)

add_library(calculator_core ${CORE_SOURCES})
target_include_directories(calculator_core PUBLIC ${COMMON_INCLUDES})
target_compile_definitions(calculator_core PUBLIC CLI_CALCULATOR_VERSION="${PROJECT_VERSION}")
target_link_libraries(calculator_core PUBLIC ZLIB::ZLIB Boost::boost)

add_executable(divisors ${TOOLS_SOURCES})
add_executable(calculator ${APP_SOURCES})

target_link_libraries(divisors PRIVATE calculator_core)
target_link_libraries(calculator PRIVATE calculator_core)
target_compile_definitions(divisors PRIVATE CLI_CALCULATOR_VERSION="${PROJECT_VERSION}")
target_compile_definitions(calculator PRIVATE CLI_CALCULATOR_VERSION="${PROJECT_VERSION}")

install(TARGETS calculator
    RUNTIME DESTINATION bin
)

if(MSVC)
  target_compile_options(divisors PRIVATE /W4)
  target_compile_options(calculator PRIVATE /W4)
endif()
option(USE_ZLIB "Enable ZLIB support" ON)

if (USE_ZLIB)
  find_package(ZLIB REQUIRED)
endif()

enable_testing()

set(CPACK_PACKAGE_NAME "cli-calculator")
set(CPACK_PACKAGE_VERSION "1.5.1")
set(CPACK_GENERATOR "DEB")

set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Farsang Benedek")
set(CPACK_DEBIAN_PACKAGE_SECTION "utils")
set(CPACK_DEBIAN_PACKAGE_DESCRIPTION "Simple command-line calculator, written in C++")

include(CPack)
