cmake_minimum_required(VERSION 2.6 FATAL_ERROR)

add_subdirectory(src)

set( TESTDATADIR "${PROJECT_SOURCE_DIR}/tests/segmenter_textwolf" )
add_test( SegmenterTextwolf src/testSegmenterTextwolf "${TESTDATADIR}/markup.xml"  "${TESTDATADIR}/markup.rul"  "${TESTDATADIR}/markup.res" "${TESTDATADIR}/markup.exp" )
add_test( SegmenterMarkupTextwolf src/testMarkupTextwolf "${PROJECT_SOURCE_DIR}/tests/segmenter_textwolf/in.xml" )

