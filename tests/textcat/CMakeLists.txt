cmake_minimum_required(VERSION 2.6 FATAL_ERROR)

add_subdirectory(src)

add_test( TextcatEnglish src/testTextcat "${PROJECT_SOURCE_DIR}/3rdParty/textcat/langclass" "${PROJECT_SOURCE_DIR}/3rdParty/textcat/langclass/conf.txt" "${PROJECT_SOURCE_DIR}/tests/textcat" english english.txt )

