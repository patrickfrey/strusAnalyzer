cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

# --------------------------------------
# SOURCES AND INCLUDES
# --------------------------------------
set( source_files
	normalizerDictMap.cpp   
)

include_directories(  
	"${PROJECT_SOURCE_DIR}/include" 
	"${CNODETRIE_INCLUDE_DIRS}"
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
add_library( strus_normalizer_dictmap SHARED ${source_files} libstrus_normalizer_dictmap.cpp )
target_link_libraries( strus_normalizer_dictmap compactnodetrie_analyzer_static strusanalyzer_private_utils strus_base )

set_target_properties(
    strus_normalizer_dictmap
    PROPERTIES
    DEBUG_POSTFIX "${CMAKE_DEBUG_POSTFIX}"
    SOVERSION "${STRUS_MAJOR_VERSION}.${STRUS_MINOR_VERSION}"
    VERSION ${STRUS_VERSION}
)


# ------------------------------
# INSTALLATION
# ------------------------------
install( TARGETS strus_normalizer_dictmap
           LIBRARY DESTINATION ${LIB_INSTALL_DIR}/strus )


