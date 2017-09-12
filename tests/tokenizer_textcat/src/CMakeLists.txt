cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

include_directories(
	"${Boost_INCLUDE_DIRS}"
	"${Intl_INCLUDE_DIRS}"
	"${PROJECT_SOURCE_DIR}/include"
	"${strusbase_INCLUDE_DIRS}"
)

link_directories(
	"${CMAKE_CURRENT_BINARY_DIR}/../../../src/analyzer"
	"${CMAKE_CURRENT_BINARY_DIR}/../../../src/textproc"
	"${CMAKE_CURRENT_BINARY_DIR}/../../../src/utils"
	"${CMAKE_CURRENT_BINARY_DIR}/../../../src/detector_std"
	"${CMAKE_CURRENT_BINARY_DIR}/../../../src/tokenizer_textcat"
	"${Boost_LIBRARY_DIRS}"
	"${strusbase_LIBRARY_DIRS}"
)

add_executable( testTokenizerTextcat testTokenizerTextcat.cpp)
target_link_libraries( testTokenizerTextcat strus_error strus_tokenizer_textcat strus_textproc strusanalyzer_private_utils ${Boost_LIBRARIES} ${Intl_LIBRARIES} )


