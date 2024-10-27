cmake_minimum_required(VERSION 3.28)
project(embedded_cpp)

set(CMAKE_CXX_STANDARD 17)

file(GLOB_RECURSE source_files
        src/**/*.cpp
)

add_library(${PROJECT_NAME} STATIC ${source_files})
target_include_directories(
        ${PROJECT_NAME} PUBLIC src
)

# Functions
function(new_target target_name target_file)
    add_executable(${target_name} ${target_file})
    target_link_libraries(${target_name} ${PROJECT_NAME})
endfunction()

# Test files
new_target(test_include test/test_include.cpp)
new_target(test_string test/test_string.cpp)
