cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

# --------------------------------------
# SOURCES AND INCLUDES
# --------------------------------------
set( source_files
	normalizerWordMap.cpp   
)

include_directories(  
	"${PROJECT_SOURCE_DIR}/include" 
	"${PROJECT_SOURCE_DIR}/3rdParty/compactNodeTrie/include"
)

link_directories(
	"${PROJECT_SOURCE_DIR}/3rdParty/compactNodeTrie/src"
	"${PROJECT_SOURCE_DIR}/src/utils"
)

# -------------------------------------------
# LIBRARY
# -------------------------------------------
add_library( strus_normalizer_wordmap SHARED ${source_files} libstrus_normalizer_wordmap.cpp )
target_link_libraries( strus_normalizer_wordmap compactnodetrie_static strus_private_utils )

set_target_properties(
    strus_normalizer_wordmap
    PROPERTIES
    DEBUG_POSTFIX "_d"
    SOVERSION "${STRUS_MAJOR_VERSION}.${STRUS_MINOR_VERSION}"
    VERSION ${STRUS_VERSION}
)


# ------------------------------
# INSTALLATION
# ------------------------------
install( TARGETS strus_normalizer_wordmap
           LIBRARY DESTINATION lib/strus )


