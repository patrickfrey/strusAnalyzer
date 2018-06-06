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
	${Boost_INCLUDE_DIRS}
	"${PROJECT_SOURCE_DIR}/include" 
	${TEXTWOLF_INCLUDE_DIRS}
	"${strusbase_INCLUDE_DIRS}"
	"${CMAKE_CURRENT_BINARY_DIR}"
)

link_directories(
	"${MAIN_SOURCE_DIR}/normalizer_snowball"
	"${MAIN_SOURCE_DIR}/normalizer_dictmap"
	"${MAIN_SOURCE_DIR}/normalizer_ngram"
	"${MAIN_SOURCE_DIR}/normalizer_charconv"
	"${MAIN_SOURCE_DIR}/normalizer_dateconv"
	"${MAIN_SOURCE_DIR}/normalizer_regex"
	"${MAIN_SOURCE_DIR}/normalizer_wordjoin"
	"${MAIN_SOURCE_DIR}/normalizer_trim"
	"${MAIN_SOURCE_DIR}/tokenizer_punctuation"
 	"${MAIN_SOURCE_DIR}/tokenizer_word"
 	"${MAIN_SOURCE_DIR}/tokenizer_regex"
 	"${MAIN_SOURCE_DIR}/tokenizer_textcat"
 	"${MAIN_SOURCE_DIR}/aggregator_vsm"
 	"${MAIN_SOURCE_DIR}/aggregator_set"
 	"${MAIN_SOURCE_DIR}/segmenter_textwolf"
 	"${MAIN_SOURCE_DIR}/segmenter_cjson"
 	"${MAIN_SOURCE_DIR}/segmenter_tsv"
 	"${MAIN_SOURCE_DIR}/segmenter_plain"
 	"${MAIN_SOURCE_DIR}/pattern"
 	"${MAIN_SOURCE_DIR}/utils"
	${Boost_LIBRARY_DIRS}
	"${strusbase_LIBRARY_DIRS}"
)


# -------------------------------------------
# LIBRARY
# -------------------------------------------
add_cppcheck( strus_textproc ${source_files} )

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
	strus_normalizer_trim
	strus_normalizer_substrindex
	strus_tokenizer_punctuation
	strus_tokenizer_word
	strus_tokenizer_regex
	strus_tokenizer_textcat
	strus_aggregator_vsm
	strus_aggregator_set
	strus_pattern_termfeeder
	strusanalyzer_private_utils
	strus_filelocator
	strus_base )

set_target_properties(
    strus_textproc
    PROPERTIES
    DEBUG_POSTFIX "${CMAKE_DEBUG_POSTFIX}"
    SOVERSION "${STRUS_MAJOR_VERSION}.${STRUS_MINOR_VERSION}"
    VERSION ${STRUS_VERSION}
)


# ------------------------------
# INSTALLATION
# ------------------------------
install( TARGETS strus_textproc
           LIBRARY DESTINATION ${LIB_INSTALL_DIR}/strus )


