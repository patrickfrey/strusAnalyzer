cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

include_directories(
	${Boost_INCLUDE_DIRS}
	"${Intl_INCLUDE_DIRS}"
	"${PROJECT_SOURCE_DIR}/include"
	"${strusbase_INCLUDE_DIRS}"
	${TESTUTILS_INCLUDE_DIRS}
)

link_directories(
	"${MAIN_LIBRARY_DIR}/analyzer"
	"${MAIN_LIBRARY_DIR}/textproc"
	"${MAIN_LIBRARY_DIR}/utils"
	"${MAIN_LIBRARY_DIR}/detector_std"
	"${MAIN_LIBRARY_DIR}/normalizer_charconv"
	"${MAIN_LIBRARY_DIR}/normalizer_dateconv"
	"${MAIN_LIBRARY_DIR}/normalizer_dictmap"
	"${MAIN_LIBRARY_DIR}/normalizer_snowball"
	"${MAIN_LIBRARY_DIR}/segmenter_textwolf"
	"${MAIN_LIBRARY_DIR}/tokenizer_punctuation"
	"${MAIN_LIBRARY_DIR}/tokenizer_word"
	${TESTUTILS_LIBRARY_DIRS}
	${Boost_LIBRARY_DIRS}
	"${strusbase_LIBRARY_DIRS}"
)

add_cppcheck( testRandomFeed testRandomFeed.cpp )

add_executable( testRandomFeed testRandomFeed.cpp)
target_link_libraries( testRandomFeed strus_stemmer strus_error strus_filelocator strus_tokenizer_word strus_tokenizer_punctuation strus_tokenizer_textcat strus_segmenter_textwolf strus_analyzer strus_normalizer_dateconv strus_normalizer_dictmap strus_detector_std strus_normalizer_charconv strus_textproc  strus_normalizer_snowball strus_aggregator_vsm strusanalyzer_private_utils ${Boost_LIBRARIES} ${Intl_LIBRARIES} )


