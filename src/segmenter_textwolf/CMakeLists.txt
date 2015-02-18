cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

# --------------------------------------
# SOURCES AND INCLUDES
# --------------------------------------
set(source_files
	segmenter.cpp
	segmenterInstance.cpp
	libstrus_segmenter_textwolf.cpp
)

include_directories(  
  "${PROJECT_SOURCE_DIR}/include" 
  "${PROJECT_SOURCE_DIR}/3rdParty/textwolf/include"
)

# -------------------------------------------
# LIBRARY
# -------------------------------------------
add_library( strus_segmenter_textwolf SHARED ${source_files} )
set_target_properties(
    strus_segmenter_textwolf
    PROPERTIES
    DEBUG_POSTFIX "_d"
    SOVERSION "${STRUS_MAJOR_VERSION}.${STRUS_MINOR_VERSION}"
    VERSION ${STRUS_VERSION}
)

# ------------------------------
# INSTALLATION
# ------------------------------
install( TARGETS strus_segmenter_textwolf
           LIBRARY DESTINATION lib/strus )
