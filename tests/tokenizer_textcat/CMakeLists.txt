cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

add_subdirectory(src)

add_test( TokenizerTextcatEnglish ${CMAKE_CURRENT_BINARY_DIR}/src/testTokenizerTextcat "${PROJECT_SOURCE_DIR}/3rdParty/textcat/langclass" "${PROJECT_SOURCE_DIR}/3rdParty/textcat/langclass/conf.txt" "${PROJECT_SOURCE_DIR}/tests/tokenizer_textcat" english english.txt )
add_test( TokenizerTextcatGerman ${CMAKE_CURRENT_BINARY_DIR}/src/testTokenizerTextcat "${PROJECT_SOURCE_DIR}/3rdParty/textcat/langclass" "${PROJECT_SOURCE_DIR}/3rdParty/textcat/langclass/conf.txt" "${PROJECT_SOURCE_DIR}/tests/tokenizer_textcat" german german.txt )

