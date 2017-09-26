cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

# --------------------------------------
# SOURCES AND INCLUDES
# --------------------------------------
set(source_files
	libstrus_tokenizer_punctuation.cpp
	punctuation_de.cpp
	punctuation_en.cpp
	punctuation.cpp
)

include_directories(  
	"${PROJECT_SOURCE_DIR}/include" 
	"${TEXTWOLF_INCLUDE_DIRS}"
	"${CNODETRIE_INCLUDE_DIRS}"
	"${Intl_INCLUDE_DIRS}"
	"${Boost_INCLUDE_DIRS}"
	"${strusbase_INCLUDE_DIRS}"
)

link_directories(
	"${CNODETRIE_LIBRARY_DIRS}"
 	"${UTILS_LIBRARY_DIRS}"
	"${Boost_LIBRARY_DIRS}"
)


# -------------------------------------------
# LIBRARY
# -------------------------------------------
add_library( strus_tokenizer_punctuation SHARED ${source_files} )
target_link_libraries( strus_tokenizer_punctuation compactnodetrie_analyzer_static strusanalyzer_private_utils )
set_target_properties(
    strus_tokenizer_punctuation
    PROPERTIES
    DEBUG_POSTFIX "_d"
    SOVERSION "${STRUS_MAJOR_VERSION}.${STRUS_MINOR_VERSION}"
    VERSION ${STRUS_VERSION}
)


# ------------------------------
# INSTALLATION
# ------------------------------
install( TARGETS strus_tokenizer_punctuation
           LIBRARY DESTINATION ${LIB_INSTALL_DIR}/strus )


