cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

# --------------------------------------
# SOURCES AND INCLUDES
# --------------------------------------
set(source_files
	testAnnotationPos.cpp
)

include_directories(  
  "${PROJECT_SOURCE_DIR}/include" 
)

link_directories(
   "${PROJECT_SOURCE_DIR}/src/segmenter_textwolf"
)


# -------------------------------------------
# LIBRARY
# -------------------------------------------
add_library( testAnnotationPos SHARED ${source_files} )
target_link_libraries( testAnnotationPos strus_segmenter_textwolf)


# ------------------------------
# INSTALLATION
# ------------------------------
install( TARGETS testAnnotationPos
           LIBRARY DESTINATION lib/strus )

