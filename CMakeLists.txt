cmake_minimum_required (VERSION 2.6)
project (DynamicBuffer)

set(CMAKE_CXX_FLAGS "-Wall -Wextra -pedantic")
set(CMAKE_CXX_STANDARD 11)

add_library(DynamicBuffer INTERFACE)
target_include_directories(DynamicBuffer INTERFACE .)

add_subdirectory(googletest)

enable_testing()
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR} .)

add_executable(Test Test.cpp)
target_link_libraries(Test gtest gtest_main DynamicBuffer)
