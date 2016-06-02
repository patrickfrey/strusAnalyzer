cmake_minimum_required(VERSION 2.6 FATAL_ERROR)

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
	"${PROJECT_SOURCE_DIR}/src/normalizer_snowball"
	"${PROJECT_SOURCE_DIR}/src/normalizer_dictmap"
	"${PROJECT_SOURCE_DIR}/src/normalizer_ngram"
	"${PROJECT_SOURCE_DIR}/src/normalizer_charconv"
	"${PROJECT_SOURCE_DIR}/src/normalizer_dateconv"
	"${PROJECT_SOURCE_DIR}/src/normalizer_regex"
	"${PROJECT_SOURCE_DIR}/src/tokenizer_punctuation"
 	"${PROJECT_SOURCE_DIR}/src/tokenizer_word"
 	"${PROJECT_SOURCE_DIR}/src/tokenizer_regex"
 	"${PROJECT_SOURCE_DIR}/src/aggregator_vsm"
 	"${PROJECT_SOURCE_DIR}/src/utils"
)


# -------------------------------------------
# LIBRARY
# -------------------------------------------
add_library( strus_textproc SHARED ${source_files} )
target_link_libraries(
	strus_textproc
	strus_detector_std
	strus_normalizer_snowball
	strus_normalizer_dictmap
	strus_normalizer_charconv
	strus_normalizer_ngram
	strus_normalizer_dateconv
	strus_normalizer_regex
	strus_tokenizer_punctuation
	strus_tokenizer_word
	strus_tokenizer_regex
	strus_aggregator_vsm
	strus_private_utils )

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


