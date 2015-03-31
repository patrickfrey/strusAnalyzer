cmake_minimum_required(VERSION 2.6 FATAL_ERROR)

# --------------------------------------
# SOURCES AND INCLUDES
# --------------------------------------
set(source_files
	normalizerDef.cpp
	documentAnalyzer.cpp
	libstrus_analyzer.cpp
	queryAnalyzer.cpp
)

include_directories(  
  "${PROJECT_SOURCE_DIR}/include" 
)

link_directories(
   "${PROJECT_SOURCE_DIR}/src/segmenter_textwolf"
   "${PROJECT_SOURCE_DIR}/src/textproc"
   "${PROJECT_SOURCE_DIR}/src/utils"
)


# -------------------------------------------
# LIBRARY
# -------------------------------------------
add_library( strus_analyzer SHARED ${source_files} )
target_link_libraries( strus_analyzer strus_segmenter_textwolf strus_textproc strus_private_utils )
set_target_properties(
    strus_analyzer
    PROPERTIES
    DEBUG_POSTFIX "_d"
    SOVERSION "${STRUS_MAJOR_VERSION}.${STRUS_MINOR_VERSION}"
    VERSION ${STRUS_VERSION}
)


# ------------------------------
# INSTALLATION
# ------------------------------
install( TARGETS strus_analyzer
           LIBRARY DESTINATION ${LIB_INSTALL_DIR}/strus )


