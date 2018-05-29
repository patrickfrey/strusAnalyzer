cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

add_subdirectory(src)

add_test( ContentStatsXML ${CMAKE_CURRENT_BINARY_DIR}/src/testContentStats  "${PROJECT_SOURCE_DIR}/tests/contentstats" application/xml UTF-8  )
add_test( ContentStatsJSON ${CMAKE_CURRENT_BINARY_DIR}/src/testContentStats  "${PROJECT_SOURCE_DIR}/tests/contentstats" application/json UTF-8  )

