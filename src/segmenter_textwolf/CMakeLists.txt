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

# ------------------------------
# INSTALLATION
# ------------------------------
install( TARGETS strus_segmenter_textwolf
           LIBRARY DESTINATION lib/strus )

