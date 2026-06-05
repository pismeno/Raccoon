# cmake/EmbedBitcode.cmake
if(NOT DEFINED INPUT_FILE OR NOT DEFINED OUTPUT_FILE)
    message(FATAL_ERROR "INPUT_FILE and OUTPUT_FILE variables must be defined.")
endif()

# Read the raw bitcode file as a hex string
file(READ "${INPUT_FILE}" hex_content HEX)

# Format the hex string into comma-separated formatting (0xXX, 0xXX, ...)
string(REGEX MATCHALL "([0-9a-fA-F][0-9a-fA-F])" hex_pairs "${hex_content}")

set(counter 0)
set(array_content "")
foreach(pair IN LISTS hex_pairs)
    string(APPEND array_content "0x${pair}, ")
    math(EXPR counter "${counter} + 1")

    # Format line breaks every 16 elements to keep the file neat
    math(EXPR line_mod "${counter} % 16")
    if(line_mod EQUAL 0)
        string(APPEND array_content "\n  ")
    endif()
endforeach()

# Write out the generated C++ file structure
file(WRITE "${OUTPUT_FILE}" "// Generated automatically by CMake. Do not edit.\n\n"
        "unsigned char runtime_bc[] = {\n  ${array_content}\n};\n\n"
        "unsigned int runtime_bc_len = ${counter};\n")