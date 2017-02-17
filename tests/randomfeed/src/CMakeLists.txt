cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

include_directories(
	"${Boost_INCLUDE_DIRS}"
	"${Intl_INCLUDE_DIRS}"
	"${PROJECT_SOURCE_DIR}/include"
	"${strusbase_INCLUDE_DIRS}"
)

link_directories(
	"${PROJECT_SOURCE_DIR}/src/analyzer"
	"${PROJECT_SOURCE_DIR}/src/textproc"
	"${PROJECT_SOURCE_DIR}/src/utils"
	"${PROJECT_SOURCE_DIR}/src/detector_std"
	"${PROJECT_SOURCE_DIR}/src/normalizer_charconv"
	"${PROJECT_SOURCE_DIR}/src/normalizer_dateconv"
	"${PROJECT_SOURCE_DIR}/src/normalizer_dictmap"
	"${PROJECT_SOURCE_DIR}/src/normalizer_snowball"
	"${PROJECT_SOURCE_DIR}/src/segmenter_textwolf"
	"${PROJECT_SOURCE_DIR}/src/tokenizer_punctuation"
	"${PROJECT_SOURCE_DIR}/src/tokenizer_word"
	"${Boost_LIBRARY_DIRS}"
	"${strusbase_LIBRARY_DIRS}"
)

add_executable( testRandomFeed testRandomFeed.cpp)
target_link_libraries( testRandomFeed strus_stemmer strus_error strus_tokenizer_word strus_tokenizer_punctuation strus_tokenizer_textcat strus_segmenter_textwolf strus_analyzer strus_normalizer_dateconv strus_normalizer_dictmap strus_detector_std strus_normalizer_charconv strus_textproc  strus_normalizer_snowball strus_aggregator_vsm strusanalyzer_private_utils ${Boost_LIBRARIES} ${Intl_LIBRARIES} )


