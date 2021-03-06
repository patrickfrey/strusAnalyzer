cmake_minimum_required(VERSION 2.8 FATAL_ERROR )

# --------------------------------------
# SOURCES AND INCLUDES
# --------------------------------------
set( source_files
	posTaggerData.cpp
	posTaggerInstance.cpp
	posTagger.cpp
	libstrus_postagger_std.cpp
)

include_directories(
	"${PROJECT_SOURCE_DIR}/include"
	"${GEN_INCLUDE_DIR}"
	"${Intl_INCLUDE_DIRS}"
        ${Boost_INCLUDE_DIRS}
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
add_cppcheck( strus_postagger_std ${source_files} )

add_library( strus_postagger_std SHARED ${source_files} )
target_link_libraries( strus_postagger_std  strus_markup_std strusanalyzer_private_utils strus_base strus_error ${Intl_LIBRARIES} )
set_target_properties(
    strus_postagger_std
    PROPERTIES
    DEBUG_POSTFIX "${CMAKE_DEBUG_POSTFIX}"
    SOVERSION "${STRUS_MAJOR_VERSION}.${STRUS_MINOR_VERSION}"
    VERSION ${STRUS_VERSION}
)

# ------------------------------
# INSTALLATION
# ------------------------------
install( TARGETS strus_postagger_std
           LIBRARY DESTINATION ${LIB_INSTALL_DIR}/strus )

