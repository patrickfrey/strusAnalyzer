cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

add_subdirectory(src)

add_test( SegmenterCJson ${CMAKE_CURRENT_BINARY_DIR}/src/testSegmenterCJson "${PROJECT_SOURCE_DIR}/tests/segmenter_cjson" )

