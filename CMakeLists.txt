cmake_minimum_required(VERSION 3.20)
project(raccoon)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(LLVM_DIR "/usr/lib/llvm-18/lib/cmake/llvm")
find_package(LLVM REQUIRED CONFIG)

find_package(ZLIB REQUIRED)
find_library(ZSTD_LIB NAMES zstd REQUIRED)

add_definitions(${LLVM_DEFINITIONS})
include_directories(${LLVM_INCLUDE_DIRS} ${ZLIB_INCLUDE_DIRS})

# =============================================================================
# AUTOMATIC RUNTIME EMBEDDING PIPELINE
# =============================================================================
set(RUNTIME_SRC "${CMAKE_CURRENT_SOURCE_DIR}/src/runtime/main.c")
set(RUNTIME_BC "${CMAKE_CURRENT_BINARY_DIR}/runtime.bc")
set(GENERATED_HEADER_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")
set(RUNTIME_HEADER "${GENERATED_HEADER_DIR}/RuntimeBitcode.hpp")

file(MAKE_DIRECTORY ${GENERATED_HEADER_DIR})

# FIX 1: Move find_program BEFORE using ${CLANG_EXE} so it isn't an empty string!
find_program(CLANG_EXE NAMES clang-18 clang REQUIRED)

# Custom rule: main.c -> runtime.bc -> RuntimeBitcode.hpp
add_custom_command(
        OUTPUT ${RUNTIME_HEADER}
        # FIX 2: Added -ffreestanding. This tells Clang it's a runtime/kernel space-like component
        # and forces it to use its own built-in internal headers (like stdint.h, stddef.h)
        # instead of relying on the host machine's glibc paths.
        COMMAND ${CLANG_EXE} -O3 -emit-llvm -ffreestanding -c ${RUNTIME_SRC} -o ${RUNTIME_BC}
        COMMAND ${CMAKE_COMMAND} -DINPUT_FILE=${RUNTIME_BC} -DOUTPUT_FILE=${RUNTIME_HEADER} -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/EmbedBitcode.cmake"
        DEPENDS ${RUNTIME_SRC}
        COMMENT "Compiling Raccoon C runtime to embedded LLVM bitcode header..."
)

add_custom_target(generate_runtime_header DEPENDS ${RUNTIME_HEADER})
# =============================================================================

# 1. Collect all .cpp files recursively from the src/compiler directory
file(GLOB_RECURSE COMPILER_SOURCES "src/compiler/*.cpp")

# 2. Add the executable using the collected list
add_executable(raccoon ${COMPILER_SOURCES})

# Establish dependency so header builds before source files try to read it
add_dependencies(raccoon generate_runtime_header)

# 3. (Optional) Helpful for many IDEs to see the headers too
file(GLOB_RECURSE COMPILER_HEADERS "src/compiler/*.hpp" "src/compiler/*.h")
target_sources(raccoon PRIVATE ${COMPILER_HEADERS})

# Add runtime to an IDE object library so CLion can provide code completion
add_library(raccoon_runtime_ide_dummy OBJECT EXCLUDE_FROM_ALL ${RUNTIME_SRC})

# Added 'linker' and 'bitreader' to components list so you can parse your embedded BC and merge modules
llvm_map_components_to_libnames(llvm_libs
        support
        core
        irreader
        executionengine
        native
        bitreader
        linker
)

target_link_libraries(raccoon
        ${llvm_libs}
        ZLIB::ZLIB
        ${ZSTD_LIB}
)

# Added your new build-generated folder to target include path
target_include_directories(raccoon PUBLIC ${LLVM_INCLUDE_DIRS} ${GENERATED_HEADER_DIR})