cmake_minimum_required(VERSION 3.16)
project(Compiler LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)

set(SOURCE_FILES
    main.cpp
    src/types.hpp
    src/types.cpp
    src/operators.hpp
    src/operators.cpp
    src/expressions.hpp
    src/expressions.cpp
    src/lexer.hpp
    src/lexer.cpp
    src/parser.hpp
    src/parser.cpp
    src/statements.hpp
    src/statements.cpp
    src/method_data.hpp
    src/method_data.cpp
    src/visitors.hpp
    src/visitors.cpp
    src/symtable.hpp
    src/symtable.cpp
)

add_executable(compiler ${SOURCE_FILES})

target_include_directories(compiler PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src
)

target_compile_options(compiler PRIVATE -Wall -Wextra -Wpedantic)
