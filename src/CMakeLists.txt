add_library(bigint SHARED "../include/bigint.hpp" "utils.hpp" "bigint.cpp" "string_parser.cpp")
target_include_directories(bigint PUBLIC "${PROJECT_SOURCE_DIR}/include" PUBLIC "${PROJECT_SOURCE_DIR}/src")

# Add some useful warnings
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(bigint PRIVATE -Wall -Wextra -Werror -Wpedantic -Wconversion -Wsign-conversion -Wshadow -Wnon-virtual-dtor)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(bigint PRIVATE /W4 /WX)
endif()
