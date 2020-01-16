cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

add_subdirectory(src)

set( TESTBIN  "${CMAKE_CURRENT_BINARY_DIR}/src/testDocTree" )
set( TESTDATADIR "${PROJECT_SOURCE_DIR}/tests/doctree" )
set( TESTOUTDIR "${CMAKE_CURRENT_BINARY_DIR}" )

add_test( DocumentTreeReadAndDump  "${TESTBIN}" "${TESTDATADIR}/input.xml"  "${TESTDATADIR}/expect.xml"  "${TESTOUTDIR}/output.xml" )


