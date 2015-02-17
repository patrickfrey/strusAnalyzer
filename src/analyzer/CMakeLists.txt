cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

# --------------------------------------
# SOURCES AND INCLUDES
# --------------------------------------
set(source_files
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
)


# -------------------------------------------
# LIBRARY
# -------------------------------------------
add_library( strus_analyzer SHARED ${source_files} )
target_link_libraries( strus_analyzer strus_segmenter_textwolf strus_textproc)


# ------------------------------
# INSTALLATION
# ------------------------------
install( TARGETS strus_analyzer
           LIBRARY DESTINATION lib/strus )


