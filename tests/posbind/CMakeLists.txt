cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

add_subdirectory(src)

add_test( PosBind ${CMAKE_CURRENT_BINARY_DIR}/src/testPosBind "${PROJECT_SOURCE_DIR}/tests/posbind/data/doc.xml" "${PROJECT_SOURCE_DIR}/tests/posbind/data/doc.res" )

