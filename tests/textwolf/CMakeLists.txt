cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

add_subdirectory(src)

set( TESTDATADIR "${PROJECT_SOURCE_DIR}/tests/textwolf" )
set( TESTEXECDIR "${CMAKE_CURRENT_BINARY_DIR}" )

add_test( EventsTextwolf ${CMAKE_CURRENT_BINARY_DIR}/src/testEventsTextwolf  ${TESTDATADIR}  ${TESTEXECDIR} )

