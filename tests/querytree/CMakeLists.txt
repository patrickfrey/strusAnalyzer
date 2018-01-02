cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

add_subdirectory(src)

add_test( RandomQueryTree src/testQueryTree "${CMAKE_CURRENT_OUTPUT_DIR}/querytree_exec" 1000 100 )

