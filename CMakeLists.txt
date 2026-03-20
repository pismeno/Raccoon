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

# 1. Collect all .cpp files recursively from the src/compiler directory
file(GLOB_RECURSE COMPILER_SOURCES "src/compiler/*.cpp")

# 2. Add the executable using the collected list
add_executable(raccoon ${COMPILER_SOURCES})

# 3. (Optional) Helpful for many IDEs to see the headers too
file(GLOB_RECURSE COMPILER_HEADERS "src/compiler/*.hpp" "src/compiler/*.h")
target_sources(raccoon PRIVATE ${COMPILER_HEADERS})

llvm_map_components_to_libnames(llvm_libs
        support
        core
        irreader
        executionengine
        native
)

target_link_libraries(raccoon
        ${llvm_libs}
        ZLIB::ZLIB
        ${ZSTD_LIB}
)

target_include_directories(raccoon PUBLIC ${LLVM_INCLUDE_DIRS})