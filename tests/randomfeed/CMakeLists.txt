cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

add_subdirectory(src)

add_test( RandomFeed ${CMAKE_CURRENT_BINARY_DIR}/src/testRandomFeed 1000 1000 "${PROJECT_SOURCE_DIR}/tests/randomfeed")

