cmake_minimum_required(VERSION 3.28)
project(embedded_cpp)

set(CMAKE_CXX_STANDARD 20)

file(GLOB_RECURSE source_files
        src/**/*.cpp
)

add_library(${PROJECT_NAME} STATIC)
target_include_directories(
        ${PROJECT_NAME} PUBLIC include
)
target_sources(${PROJECT_NAME} PRIVATE ${source_files})

# Functions
function(new_target target_name target_file)
    add_executable(${target_name} ${sources} ${target_file})
    target_link_libraries(${target_name} ${PROJECT_NAME})
endfunction()

# Test files
new_target(test_include test/test_include.cpp)


