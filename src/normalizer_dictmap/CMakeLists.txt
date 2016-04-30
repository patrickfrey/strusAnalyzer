cmake_minimum_required(VERSION 2.6 FATAL_ERROR)

# --------------------------------------
# SOURCES AND INCLUDES
# --------------------------------------
set( source_files
	normalizerDictMap.cpp   
)

include_directories(  
	"${PROJECT_SOURCE_DIR}/include" 
	"${PROJECT_SOURCE_DIR}/3rdParty/compactNodeTrie/include"
	"${Intl_INCLUDE_DIRS}"
	"${Boost_INCLUDE_DIRS}"
	"${strusbase_INCLUDE_DIRS}"
)

link_directories(
	"${PROJECT_SOURCE_DIR}/3rdParty/compactNodeTrie/src"
	"${PROJECT_SOURCE_DIR}/src/utils"
)

# -------------------------------------------
# LIBRARY
# -------------------------------------------
add_library( strus_normalizer_dictmap SHARED ${source_files} libstrus_normalizer_dictmap.cpp )
target_link_libraries( strus_normalizer_dictmap compactnodetrie_static strus_private_utils )

set_target_properties(
    strus_normalizer_dictmap
    PROPERTIES
    DEBUG_POSTFIX "_d"
    SOVERSION "${STRUS_MAJOR_VERSION}.${STRUS_MINOR_VERSION}"
    VERSION ${STRUS_VERSION}
)


# ------------------------------
# INSTALLATION
# ------------------------------
install( TARGETS strus_normalizer_dictmap
           LIBRARY DESTINATION ${LIB_INSTALL_DIR}/strus )


