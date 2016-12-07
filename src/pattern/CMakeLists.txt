cmake_minimum_required(VERSION 2.6 FATAL_ERROR)

# --------------------------------------
# SOURCES AND INCLUDES
# --------------------------------------
set( source_files
)

include_directories(  
	"${PROJECT_SOURCE_DIR}/include" 
	"${Intl_INCLUDE_DIRS}"
 	"${Boost_INCLUDE_DIRS}"
	"${strusbase_INCLUDE_DIRS}"
)

link_directories(
	"${strusbase_LIBRARY_DIRS}"
)


# -------------------------------------------
# LIBRARY
# -------------------------------------------
add_library( strus_pattern_termfeeder SHARED  libstrus_pattern_termfeeder.cpp  ${source_files} )
target_link_libraries( strus_pattern_termfeeder  strus_private_utils strus_base )
set_target_properties(
    strus_pattern_termfeeder
    PROPERTIES
    DEBUG_POSTFIX "_d"
    SOVERSION "${STRUS_MAJOR_VERSION}.${STRUS_MINOR_VERSION}"
    VERSION ${STRUS_VERSION}
)

# ------------------------------
# INSTALLATION
# ------------------------------
install( TARGETS strus_pattern_termfeeder
           LIBRARY DESTINATION ${LIB_INSTALL_DIR}/strus )


