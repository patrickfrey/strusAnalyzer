cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

# --------------------------------------
# SOURCES AND INCLUDES
# --------------------------------------
set(source_files
	tokenizerRegex.cpp
)

include_directories(  
	"${Intl_INCLUDE_DIRS}"
	${Boost_INCLUDE_DIRS}
	"${PROJECT_SOURCE_DIR}/include" 
	"${TEXTWOLF_INCLUDE_DIRS}"
	"${strusbase_INCLUDE_DIRS}"
)

link_directories(
	"${UTILS_LIBRARY_DIRS}"
	${Boost_LIBRARY_DIRS}
	"${strusbase_LIBRARY_DIRS}"
)

# -------------------------------------------
# LIBRARY
# -------------------------------------------
add_library( strus_tokenizer_regex SHARED libstrus_tokenizer_regex.cpp ${source_files} )
target_link_libraries( strus_tokenizer_regex strusanalyzer_private_utils strus_base )

set_target_properties(
    strus_tokenizer_regex
    PROPERTIES
    DEBUG_POSTFIX "${CMAKE_DEBUG_POSTFIX}"
    SOVERSION "${STRUS_MAJOR_VERSION}.${STRUS_MINOR_VERSION}"
    VERSION ${STRUS_VERSION}
)

# ------------------------------
# INSTALLATION
# ------------------------------
install( TARGETS strus_tokenizer_regex
           LIBRARY DESTINATION ${LIB_INSTALL_DIR}/strus )


