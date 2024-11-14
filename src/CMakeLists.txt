file(GLOB_RECURSE SOURCES "*.cpp" "*.hpp" "${PROJECT_SOURCE_DIR}/include/*.hpp")

add_library(bigint SHARED ${SOURCES})
target_include_directories(bigint PUBLIC "${PROJECT_SOURCE_DIR}/include" PUBLIC "${PROJECT_SOURCE_DIR}/src")

# Add some useful warnings
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(bigint PRIVATE -Wall -Wextra -Werror -Wpedantic -Wconversion -Wsign-conversion -Wshadow
        -Wnon-virtual-dtor -Wimplicit-fallthrough)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(bigint PRIVATE /W4 /WX)
endif()

set_target_properties(bigint PROPERTIES CXX_CLANG_TIDY "clang-tidy")
