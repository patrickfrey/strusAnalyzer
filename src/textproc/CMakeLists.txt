cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

# --------------------------------------
# SOURCES AND INCLUDES
# --------------------------------------
set(source_files
	libstrus_textproc.cpp
	textProcessor.cpp
)

include_directories(  
	"${Intl_INCLUDE_DIRS}"
	"${Boost_INCLUDE_DIRS}"
	"${PROJECT_SOURCE_DIR}/include" 
	"${PROJECT_SOURCE_DIR}/3rdParty/textwolf/include"
	"${strusbase_INCLUDE_DIRS}"
)

link_directories(
	"${CMAKE_CURRENT_BINARY_DIR}/../src/normalizer_snowball"
	"${CMAKE_CURRENT_BINARY_DIR}/../src/normalizer_dictmap"
	"${CMAKE_CURRENT_BINARY_DIR}/../src/normalizer_ngram"
	"${CMAKE_CURRENT_BINARY_DIR}/../src/normalizer_charconv"
	"${CMAKE_CURRENT_BINARY_DIR}/../src/normalizer_dateconv"
	"${CMAKE_CURRENT_BINARY_DIR}/../src/normalizer_regex"
	"${CMAKE_CURRENT_BINARY_DIR}/../src/normalizer_wordjoin"
	"${CMAKE_CURRENT_BINARY_DIR}/../src/tokenizer_punctuation"
 	"${CMAKE_CURRENT_BINARY_DIR}/../src/tokenizer_word"
 	"${CMAKE_CURRENT_BINARY_DIR}/../src/tokenizer_regex"
 	"${CMAKE_CURRENT_BINARY_DIR}/../src/tokenizer_textcat"
 	"${CMAKE_CURRENT_BINARY_DIR}/../src/aggregator_vsm"
 	"${CMAKE_CURRENT_BINARY_DIR}/../src/aggregator_set"
 	"${CMAKE_CURRENT_BINARY_DIR}/../src/segmenter_textwolf"
 	"${CMAKE_CURRENT_BINARY_DIR}/../src/segmenter_cjson"
 	"${CMAKE_CURRENT_BINARY_DIR}/../src/segmenter_tsv"
 	"${CMAKE_CURRENT_BINARY_DIR}/../src/segmenter_plain"
 	"${CMAKE_CURRENT_BINARY_DIR}/../src/pattern"
 	"${CMAKE_CURRENT_BINARY_DIR}/../src/utils"
	"${Boost_LIBRARY_DIRS}"
	"${strusbase_LIBRARY_DIRS}"
)


# -------------------------------------------
# LIBRARY
# -------------------------------------------
add_library( strus_textproc SHARED ${source_files} )
target_link_libraries(
	strus_textproc
	strus_detector_std
	strus_segmenter_textwolf
	strus_segmenter_cjson
	strus_segmenter_tsv
	strus_segmenter_plain
	strus_normalizer_snowball
	strus_normalizer_dictmap
	strus_normalizer_charconv
	strus_normalizer_ngram
	strus_normalizer_dateconv
	strus_normalizer_regex
	strus_normalizer_wordjoin
	strus_tokenizer_punctuation
	strus_tokenizer_word
	strus_tokenizer_regex
	strus_tokenizer_textcat
	strus_aggregator_vsm
	strus_aggregator_set
	strus_pattern_termfeeder
	strusanalyzer_private_utils )

set_target_properties(
    strus_textproc
    PROPERTIES
    DEBUG_POSTFIX "_d"
    SOVERSION "${STRUS_MAJOR_VERSION}.${STRUS_MINOR_VERSION}"
    VERSION ${STRUS_VERSION}
)


# ------------------------------
# INSTALLATION
# ------------------------------
install( TARGETS strus_textproc
           LIBRARY DESTINATION ${LIB_INSTALL_DIR}/strus )


