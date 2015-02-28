cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

# --------------------------------------
# SOURCES AND INCLUDES
# --------------------------------------
include_directories(  
  "${PROJECT_SOURCE_DIR}/include" 
)

link_directories(
   "${PROJECT_SOURCE_DIR}/src/segmenter_textwolf"
)


# -------------------------------------------
# LIBRARY
# -------------------------------------------
add_executable( testAnnotationPos testAnnotationPos.cpp)
target_link_libraries( testAnnotationPos strus_segmenter_textwolf)

