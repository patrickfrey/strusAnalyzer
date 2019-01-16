cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

add_subdirectory(src)

add_test( SplitCJsonLs "${CMAKE_CURRENT_BINARY_DIR}/src/testSplitJsonLs"  "${CMAKE_CURRENT_BINARY_DIR}"  "${PROJECT_SOURCE_DIR}/tests/splitjsonls/input.jsonl" "${PROJECT_SOURCE_DIR}/tests/splitjsonls/expect.jsonl" )

