cmake_minimum_required (VERSION 2.6)
project (DynamicBuffer)

set(CMAKE_CXX_FLAGS "-Wall -Wextra -pedantic")
set(CMAKE_CXX_STANDARD 11)

add_library(DynamicBuffer INTERFACE)
target_include_directories(DynamicBuffer INTERFACE .)

if (TEST)
add_subdirectory(googletest)

enable_testing()
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR} .)

add_executable(${PROJECT_NAME}_Test Test.cpp)
target_link_libraries(${PROJECT_NAME}_Test gtest gtest_main DynamicBuffer)
endif(TEST)
