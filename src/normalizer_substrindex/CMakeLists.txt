cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

# --------------------------------------
# SOURCES AND INCLUDES
# --------------------------------------
set( source_files
	normalizer_substrindex.cpp   
)

include_directories(  
	"${PROJECT_SOURCE_DIR}/include" 
	"${CNODETRIE_INCLUDE_DIRS}"
	${TEXTWOLF_INCLUDE_DIRS}
	"${Intl_INCLUDE_DIRS}"
	${Boost_INCLUDE_DIRS}
	"${strusbase_INCLUDE_DIRS}"
)

link_directories(
	"${CNODETRIE_LIBRARY_DIRS}"
	"${UTILS_LIBRARY_DIRS}"
	"${strusbase_LIBRARY_DIRS}"
	${Boost_LIBRARY_DIRS}
)

# -------------------------------------------
# LIBRARY
# -------------------------------------------
add_cppcheck( strus_normalizer_substrindex ${source_files} libstrus_normalizer_substrindex.cpp )

add_library( strus_normalizer_substrindex SHARED ${source_files} libstrus_normalizer_substrindex.cpp )
target_link_libraries( strus_normalizer_substrindex compactnodetrie_analyzer_static strusanalyzer_private_utils strus_base )

set_target_properties(
    strus_normalizer_substrindex
    PROPERTIES
    DEBUG_POSTFIX "${CMAKE_DEBUG_POSTFIX}"
    SOVERSION "${STRUS_MAJOR_VERSION}.${STRUS_MINOR_VERSION}"
    VERSION ${STRUS_VERSION}
)


# ------------------------------
# INSTALLATION
# ------------------------------
install( TARGETS strus_normalizer_substrindex
           LIBRARY DESTINATION ${LIB_INSTALL_DIR}/strus )


