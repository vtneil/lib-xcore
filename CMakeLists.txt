cmake_minimum_required(VERSION 3.25)
project(lib_xcore)

set(CMAKE_CXX_STANDARD 17)

file(GLOB_RECURSE source_files
        src/**/*.cpp
)

add_library(${PROJECT_NAME} STATIC ${source_files})
target_include_directories(${PROJECT_NAME} PUBLIC src)

# Functions
function(new_target target_name target_file)
    add_executable(${target_name} ${target_file})
    target_link_libraries(${target_name} ${PROJECT_NAME})
endfunction()

# Test files
new_target(test_include test/test_include.cpp)
new_target(test_buffer test/test_buffer.cpp)
new_target(test_deque test/test_deque.cpp)
new_target(test_cache test/test_cache.cpp)
new_target(test_string test/test_string.cpp)
new_target(test_tuple test/test_tuple.cpp)
new_target(test_vector test/test_vector.cpp)
new_target(test_random test/test_random.cpp)
new_target(test_variant test/test_variant.cpp)
new_target(test_pipeline test/test_pipeline.cpp)
new_target(bench_random benchmark/bench_random.cpp)
