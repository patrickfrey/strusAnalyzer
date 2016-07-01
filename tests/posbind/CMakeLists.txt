cmake_minimum_required(VERSION 2.6 FATAL_ERROR)

add_subdirectory(src)

add_test( PosBind src/testPosBind "${PROJECT_SOURCE_DIR}/tests/posbind/data/doc.xml" "${PROJECT_SOURCE_DIR}/tests/posbind/data/doc.res" )

