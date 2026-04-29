cmake_minimum_required(VERSION 3.16)
project(Compiler LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)

set(SOURCE_FILES
    src/main.cpp
    src/core/types.hpp
    src/core/types.cpp
    src/core/operators.hpp
    src/core/operators.cpp
    src/core/expressions.hpp
    src/core/expressions.cpp
    src/core/lexer.hpp
    src/core/lexer.cpp
    src/core/parser.hpp
    src/core/parser.cpp
    src/core/statements.hpp
    src/core/statements.cpp
    src/core/utils.hpp
    src/core/utils.cpp
    src/core/visitors.hpp
    src/core/visitors.cpp
    src/core/symtable.hpp
    src/core/symtable.cpp
)

add_executable(compiler ${SOURCE_FILES})

target_include_directories(compiler PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/core
)

target_compile_options(compiler PRIVATE -Wall -Wextra -Wpedantic)
