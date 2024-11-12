cmake_minimum_required(VERSION 3.30)

# If compiler is not explicitly specified by user, Use Clang on Unix when available, otherwise use GCC.
# This must be done before the project() command because project() sets the compiler.
if(UNIX)
    if(NOT DEFINED CMAKE_CXX_COMPILER)
        find_program(CLANG_PATH clang++)
        find_program(GCC_PATH g++)
        if(CLANG_PATH)
            message(STATUS "Found Clang: ${CLANG_PATH}")
            set(CMAKE_CXX_COMPILER ${CLANG_PATH})
        elseif(GCC_PATH)
            message(STATUS "Found GCC: ${GCC_PATH}")
            set(CMAKE_CXX_COMPILER ${GCC_PATH})
        else()
            message(FATAL_ERROR "Neither Clang nor GCC was found. Please install at least one of them.")
        endif()
    else()
        message(STATUS "Using user-specified compiler: ${CMAKE_CXX_COMPILER}")
    endif()
elseif(NOT WIN32)
    message(FATAL_ERROR "Unsupported platform")
endif()

project(BigInt VERSION 0.1 LANGUAGES CXX)

# Use C++23
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Generate compile_commands.json for clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Add cmake/ to CMAKE_MODULE_PATH
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

add_subdirectory(src)

enable_testing()
add_subdirectory(test)
