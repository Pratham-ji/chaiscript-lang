cmake_minimum_required(VERSION 3.20)
project(ChaiScript VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Production-grade warning discipline
add_compile_options(-Wall -Wextra -Wpedantic -Werror=return-type)

# Core compiler library (statically linked for single-binary distribution)
add_library(chai_core STATIC
    src/Lexer.cpp
    src/Parser.cpp
    src/Sema.cpp
    src/CodeGen.cpp
)
target_include_directories(chai_core PUBLIC include)

# Compiler executable
add_executable(chaiscript src/main.cpp)
target_link_libraries(chaiscript PRIVATE chai_core)
