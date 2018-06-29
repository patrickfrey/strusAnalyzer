cmake_minimum_required(VERSION 2.8 FATAL_ERROR )

# --------------------------------------
# SOURCES AND INCLUDES
# --------------------------------------
set( source_files
	strusXmlPosTagger.cpp
)

include_directories(
	    "${Intl_INCLUDE_DIRS}"
            ${Boost_INCLUDE_DIRS}
	    "${UTILS_INCLUDE_DIRS}"
	    "${TEXTWOLF_INCLUDE_DIRS}"
	    "${strusbase_INCLUDE_DIRS}"
)
link_directories( 
	   "${UTILS_LIBRARY_DIRS}" 
	   ${Boost_LIBRARY_DIRS} 
	   "${strusbase_LIBRARY_DIRS}"
)

# ------------------------------
# PROGRAMS
# ------------------------------
add_cppcheck( strusXmlPosTagger ${source_files} )

add_executable( strusXmlPosTagger ${source_files} )
target_link_libraries( strusXmlPosTagger  strusanalyzer_private_utils strus_base strus_error ${Intl_LIBRARIES})

# ------------------------------
# INSTALLATION
# ------------------------------
install( TARGETS strusXmlPosTagger
	   RUNTIME DESTINATION bin )

