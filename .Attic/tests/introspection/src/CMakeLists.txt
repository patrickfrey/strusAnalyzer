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
	"${MAIN_LIBRARY_DIR}/normalizer_ngram"
	"${MAIN_LIBRARY_DIR}/normalizer_regex"
	"${MAIN_LIBRARY_DIR}/normalizer_snowball"
	"${MAIN_LIBRARY_DIR}/normalizer_trim"
	"${MAIN_LIBRARY_DIR}/normalizer_wordjoin"
	"${MAIN_LIBRARY_DIR}/segmenter_textwolf"
	"${MAIN_LIBRARY_DIR}/segmenter_cjson"
	"${MAIN_LIBRARY_DIR}/segmenter_plain"
	"${MAIN_LIBRARY_DIR}/segmenter_tsv"
	"${MAIN_LIBRARY_DIR}/tokenizer_punctuation"
	"${MAIN_LIBRARY_DIR}/tokenizer_word"
	"${MAIN_LIBRARY_DIR}/tokenizer_regex"
	"${MAIN_LIBRARY_DIR}/tokenizer_textcat"
	"${MAIN_LIBRARY_DIR}/aggregator_set"
	"${MAIN_LIBRARY_DIR}/aggregator_vsm"
	${TESTUTILS_LIBRARY_DIRS}
	${Boost_LIBRARY_DIRS}
	"${strusbase_LIBRARY_DIRS}"
)

add_cppcheck( testIntrospection testIntrospection.cpp )

set( TEXTPROC_FUNCTION_LIBS strus_aggregator_set strus_aggregator_vsm strus_tokenizer_punctuation strus_tokenizer_regex strus_tokenizer_textcat strus_tokenizer_word strus_normalizer_charconv strus_normalizer_dateconv strus_normalizer_dictmap strus_normalizer_ngram strus_normalizer_regex strus_normalizer_snowball strus_normalizer_trim strus_normalizer_wordjoin strus_segmenter_cjson strus_segmenter_plain strus_segmenter_textwolf strus_segmenter_tsv strus_segmenter_utils strus_detector_std )

add_executable( testIntrospection testIntrospection.cpp)
target_link_libraries( testIntrospection strus_analyzer strus_textproc ${TEXTPROC_FUNCTION_LIBS} strusanalyzer_private_utils strus_error ${Boost_LIBRARIES} ${Intl_LIBRARIES} )


