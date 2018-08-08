cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

# --------------------------------------
# SOURCES AND INCLUDES
# --------------------------------------
set(source_files
	patternMatchProgramParser.cpp
	programLoader.cpp
	libstrus_analyzer_prgload_std.cpp
)

include_directories(  
	"${Intl_INCLUDE_DIRS}"
	${Boost_INCLUDE_DIRS}
	"${PROJECT_SOURCE_DIR}/include" 
	"${strusbase_INCLUDE_DIRS}"
)

link_directories(
	"${strusbase_LIBRARY_DIRS}"
	"${UTILS_LIBRARY_DIRS}"
	${Boost_LIBRARY_DIRS}
)

# -------------------------------------------
# LIBRARY
# -------------------------------------------
add_cppcheck( strus_analyzer_prgload_std ${source_files} )

add_library( strus_analyzer_prgload_std SHARED ${source_files} )
target_link_libraries( strus_analyzer_prgload_std strus_pattern_serialize strusanalyzer_private_utils strus_base )
set_target_properties(
    strus_analyzer_prgload_std
    PROPERTIES
    DEBUG_POSTFIX "${CMAKE_DEBUG_POSTFIX}"
    SOVERSION "${STRUS_MAJOR_VERSION}.${STRUS_MINOR_VERSION}"
    VERSION ${STRUS_VERSION}
)

# ------------------------------
# INSTALLATION
# ------------------------------
install( TARGETS strus_analyzer_prgload_std
           LIBRARY DESTINATION ${LIB_INSTALL_DIR}/strus )

