cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

# --------------------------------------
# SOURCES AND INCLUDES
# --------------------------------------
set(source_files
	segmenter.cpp
	libstrus_segmenter_textwolf.cpp
)

include_directories(  
	"${Intl_INCLUDE_DIRS}"
	${Boost_INCLUDE_DIRS}
	"${PROJECT_SOURCE_DIR}/include" 
	${TEXTWOLF_INCLUDE_DIRS}
	"${strusbase_INCLUDE_DIRS}"
)
link_directories(
	"${MAIN_SOURCE_DIR}/segmenter_utils" 
	"${strusbase_LIBRARY_DIRS}"
	${Boost_LIBRARY_DIRS}
)

# -------------------------------------------
# LIBRARY
# -------------------------------------------
add_cppcheck( strus_segmenter_textwolf ${source_files} )

add_library( strus_segmenter_textwolf SHARED ${source_files} )
target_link_libraries( strus_segmenter_textwolf strus_segmenter_utils strusanalyzer_private_utils strus_base )
set_target_properties(
    strus_segmenter_textwolf
    PROPERTIES
    DEBUG_POSTFIX "${CMAKE_DEBUG_POSTFIX}"
    SOVERSION "${STRUS_MAJOR_VERSION}.${STRUS_MINOR_VERSION}"
    VERSION ${STRUS_VERSION}
)

# ------------------------------
# INSTALLATION
# ------------------------------
install( TARGETS strus_segmenter_textwolf
           LIBRARY DESTINATION ${LIB_INSTALL_DIR}/strus )

