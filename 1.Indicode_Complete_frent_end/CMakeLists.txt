cmake_minimum_required(VERSION 3.20)
project(indicode_frontend VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(indicode_frontend
    src/lexer.cpp
    src/parser.cpp
    src/sema.cpp
    src/ast_printer.cpp
)
target_include_directories(indicode_frontend PUBLIC include)
target_compile_options(indicode_frontend PRIVATE -Wall -Wextra -Wpedantic)

add_executable(indicode src/driver/main.cpp)
target_link_libraries(indicode PRIVATE indicode_frontend)

include(CTest)
if(BUILD_TESTING)
    add_executable(indicode_lexer_tests tests/lexer_tests.cpp)
    target_link_libraries(indicode_lexer_tests PRIVATE indicode_frontend)
    add_test(NAME lexer_tests COMMAND indicode_lexer_tests)

    add_executable(indicode_parser_tests tests/parser_tests.cpp)
    target_link_libraries(indicode_parser_tests PRIVATE indicode_frontend)
    add_test(NAME parser_tests COMMAND indicode_parser_tests)

    add_executable(indicode_sema_tests tests/sema_tests.cpp)
    target_link_libraries(indicode_sema_tests PRIVATE indicode_frontend)
    add_test(NAME sema_tests COMMAND indicode_sema_tests)

    add_test(NAME cli_check COMMAND indicode check ${CMAKE_CURRENT_SOURCE_DIR}/tests/fixtures/hello.indi)
    add_test(NAME cli_rejects_type_error COMMAND indicode check ${CMAKE_CURRENT_SOURCE_DIR}/tests/fixtures/type_error.indi)
    set_tests_properties(cli_rejects_type_error PROPERTIES WILL_FAIL TRUE)
endif()