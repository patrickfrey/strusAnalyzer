cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

add_subdirectory(src)

set( TESTDATADIR "${PROJECT_SOURCE_DIR}/tests/segmenter_textwolf" )
add_test( MarkupTextwolf ${CMAKE_CURRENT_BINARY_DIR}/src/testMarkupTextwolf  "${TESTDATADIR}/markup.xml"  "${TESTDATADIR}/markup.rul"  "${TESTDATADIR}/markup.res" "${TESTDATADIR}/markup.exp" )
add_test( SegmenterTextwolf ${CMAKE_CURRENT_BINARY_DIR}/src/testSegmenterTextwolf "${PROJECT_SOURCE_DIR}/tests/segmenter_textwolf/" )

