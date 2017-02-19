cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

add_subdirectory(src)

add_test( SegmenterTsv src/testSegmenterTsv "${PROJECT_SOURCE_DIR}/tests/segmenter_tsv" )
