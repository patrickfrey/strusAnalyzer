cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

add_subdirectory(src)

add_test( Introspection ${CMAKE_CURRENT_BINARY_DIR}/src/testIntrospection "${PROJECT_SOURCE_DIR}/tests/introspection"  "${TEXTCAT_CONFIG_DIR}" ) 

