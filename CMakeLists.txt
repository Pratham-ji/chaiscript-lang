cmake_minimum_required(VERSION 3.16)
project(ChaiScript VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ── Core compiler library (statically linked for single-binary distribution) ──
add_library(chai_core STATIC
    src/Lexer.cpp
    src/Parser.cpp
    src/Sema.cpp
    src/CodeGen.cpp
)
target_include_directories(chai_core PUBLIC include)

# Production-grade warning discipline — cross-platform
if(MSVC)
    target_compile_options(chai_core PRIVATE /W4 /permissive-)
else()
    target_compile_options(chai_core PRIVATE -Wall -Wextra -Wpedantic -Werror=return-type)
endif()

# ── Compiler executable ──
add_executable(chaiscript src/main.cpp)
target_link_libraries(chaiscript PRIVATE chai_core)

if(MSVC)
    target_compile_options(chaiscript PRIVATE /W4 /permissive-)
else()
    target_compile_options(chaiscript PRIVATE -Wall -Wextra -Wpedantic -Werror=return-type)
endif()

# ── Install target (optional, for system-wide installation) ──
install(TARGETS chaiscript RUNTIME DESTINATION bin)
