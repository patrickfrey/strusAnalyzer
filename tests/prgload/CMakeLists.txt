cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

add_subdirectory(src)

add_test( DocumentAnalyzerLoadProgram  ${CMAKE_CURRENT_BINARY_DIR}/src/testDocumentAnalyzerLoadProgram "${PROJECT_SOURCE_DIR}/tests/prgload"  doc.ana input.xml expect.txt )
