Include(FetchContent)

file(GLOB_RECURSE BENCHMARK_SOURCES CONFIGURE_DEPENDS "*.cpp")

FetchContent_Declare(
    GoogleBenchmark
    GIT_REPOSITORY https://github.com/google/benchmark.git
    GIT_TAG        v1.9.0
)

set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "Disable Google Test in Google Benchmark")

FetchContent_MakeAvailable(GoogleBenchmark)

add_executable(bigint_benchmark ${BENCHMARK_SOURCES})
target_include_directories(bigint_benchmark PRIVATE ${CMAKE_CURRENT_LIST_DIR}/include)
target_link_libraries(bigint_benchmark PRIVATE benchmark::benchmark)
target_link_libraries(bigint_benchmark PRIVATE bigint)
