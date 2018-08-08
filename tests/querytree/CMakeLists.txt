cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

add_subdirectory(src)

add_test( RandomQueryTree ${CMAKE_CURRENT_BINARY_DIR}/src/testQueryTree "${CMAKE_CURRENT_BINARY_DIR}/querytree_exec" 400 10000 )
