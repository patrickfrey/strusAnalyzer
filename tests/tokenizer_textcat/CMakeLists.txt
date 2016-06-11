cmake_minimum_required(VERSION 2.6 FATAL_ERROR)

add_subdirectory(src)

add_test( TokenizerTextcatEnglish src/testTokenizerTextcat "${PROJECT_SOURCE_DIR}/3rdParty/textcat/langclass" "${PROJECT_SOURCE_DIR}/3rdParty/textcat/langclass/conf.txt" "${PROJECT_SOURCE_DIR}/tests/tokenizer_textcat" english english.txt )
add_test( TokenizerTextcatGerman src/testTokenizerTextcat "${PROJECT_SOURCE_DIR}/3rdParty/textcat/langclass" "${PROJECT_SOURCE_DIR}/3rdParty/textcat/langclass/conf.txt" "${PROJECT_SOURCE_DIR}/tests/tokenizer_textcat" german german.txt )

