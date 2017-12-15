cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

# --------------------------------------
# SOURCES AND INCLUDES
# --------------------------------------
set(source_files
	xpathAutomaton.cpp
)

include_directories(  
	"${Intl_INCLUDE_DIRS}"
	${Boost_INCLUDE_DIRS}
	"${PROJECT_SOURCE_DIR}/include" 
	"${TEXTWOLF_INCLUDE_DIRS}"
	"${strusbase_INCLUDE_DIRS}"
)

link_directories(
	"${UTILS_LIBRARY_DIRS}"
	"${strusbase_LIBRARY_DIRS}"
	${Boost_LIBRARY_DIRS}
)

# -------------------------------------------
# LIBRARY
# -------------------------------------------
add_library( strus_segmenter_utils STATIC ${source_files} )
target_link_libraries( strus_segmenter_utils strusanalyzer_private_utils strus_base )
set_property( TARGET strus_segmenter_utils PROPERTY POSITION_INDEPENDENT_CODE TRUE )


