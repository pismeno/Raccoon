cmake_minimum_required(VERSION 3.20)
project(Raccoon)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(LLVM_DIR "/usr/lib/llvm-18/lib/cmake/llvm")
find_package(LLVM REQUIRED CONFIG)

find_package(ZLIB REQUIRED)
find_library(ZSTD_LIB NAMES zstd REQUIRED)

add_definitions(${LLVM_DEFINITIONS})
include_directories(${LLVM_INCLUDE_DIRS} ${ZLIB_INCLUDE_DIRS})

add_executable(Raccoon
        src/compiler/main.cpp
        src/compiler/src/ast/AST.cpp
        src/compiler/src/ast/Printer.cpp
        src/compiler/src/ast/SemanticAnalyzer.cpp
        src/compiler/src/ast/Expression.cpp
        src/compiler/src/Lexer.cpp
        src/compiler/src/Parser.cpp
)

llvm_map_components_to_libnames(llvm_libs
        support
        core
        irreader
        executionengine
        native
)

target_link_libraries(Raccoon
        ${llvm_libs}
        ZLIB::ZLIB
        ${ZSTD_LIB}
)

target_include_directories(Raccoon PUBLIC ${LLVM_INCLUDE_DIRS})