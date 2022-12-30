
find_package(benchmark)

if (benchmark_FOUND)
    message("Found benchmark.")
    add_executable(perf_test ./perf.cpp ${CMAKE_SOURCE_DIR}/src/si/tasks.cpp)
    target_link_libraries(perf_test benchmark::benchmark)
    # target_include_directories(perf_test "${benchmark_INCLUDE_DIRS}")
else()
    message("benchmark is not found. Relative binary will not be build.")
endif()
