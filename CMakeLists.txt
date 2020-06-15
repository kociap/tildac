cmake_minimum_required(VERSION 3.13)

project(crust CXX)

add_executable(crust
    "${CMAKE_CURRENT_SOURCE_DIR}/compiler/tildac/ast.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/compiler/tildac/ast_printing.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/compiler/tildac/ast_printing.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/compiler/tildac/main.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/compiler/tildac/parser.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/compiler/tildac/parser.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/compiler/tildac/types.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/compiler/tildac/utility.hpp"
)
set_target_properties(crust PROPERTIES CXX_STANDARD 17 CXX_EXTENSIONS OFF)
target_include_directories(crust PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/compiler")
target_compile_definitions(crust PRIVATE _CRT_SECURE_NO_WARNINGS)
