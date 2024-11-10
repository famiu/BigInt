add_library(bigint SHARED "../include/bigint.hpp" "bigint.cpp")
target_include_directories(bigint PUBLIC "${PROJECT_SOURCE_DIR}/include")
