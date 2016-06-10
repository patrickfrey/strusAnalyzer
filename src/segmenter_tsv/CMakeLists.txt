cmake_minimum_required(VERSION 2.6 FATAL_ERROR)

# --------------------------------------
# SOURCES AND INCLUDES
# --------------------------------------
set(source_files
	tsvSegmenter.cpp
	libstrus_segmenter_tsv.cpp
)

include_directories(  
	"${Intl_INCLUDE_DIRS}"
	"${Boost_INCLUDE_DIRS}"
	"${PROJECT_SOURCE_DIR}/include" 
	"${strusbase_INCLUDE_DIRS}"
)

# -------------------------------------------
# LIBRARY
# -------------------------------------------
add_library( strus_segmenter_tsv SHARED ${source_files} )
target_link_libraries( strus_segmenter_tsv strus_segmenter_utils strus_private_utils )
set_target_properties(
    strus_segmenter_tsv
    PROPERTIES
    DEBUG_POSTFIX "_d"
    SOVERSION "${STRUS_MAJOR_VERSION}.${STRUS_MINOR_VERSION}"
    VERSION ${STRUS_VERSION}
)

# ------------------------------
# INSTALLATION
# ------------------------------
install( TARGETS strus_segmenter_tsv
           LIBRARY DESTINATION ${LIB_INSTALL_DIR}/strus )

