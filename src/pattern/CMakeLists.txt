cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

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
target_link_libraries( strus_pattern_termfeeder  strusanalyzer_private_utils strus_base )
set_target_properties(
    strus_pattern_termfeeder
    PROPERTIES
    DEBUG_POSTFIX "${CMAKE_DEBUG_POSTFIX}"
    SOVERSION "${STRUS_MAJOR_VERSION}.${STRUS_MINOR_VERSION}"
    VERSION ${STRUS_VERSION}
)

add_library( strus_pattern_serialize SHARED  libstrus_pattern_serialize.cpp  ${source_files} )
target_link_libraries( strus_pattern_serialize  strusanalyzer_private_utils strus_base )
set_target_properties(
    strus_pattern_serialize
    PROPERTIES
    DEBUG_POSTFIX "${CMAKE_DEBUG_POSTFIX}"
    SOVERSION "${STRUS_MAJOR_VERSION}.${STRUS_MINOR_VERSION}"
    VERSION ${STRUS_VERSION}
)

# ------------------------------
# INSTALLATION
# ------------------------------
install( TARGETS strus_pattern_termfeeder
           LIBRARY DESTINATION ${LIB_INSTALL_DIR}/strus )
install( TARGETS strus_pattern_serialize
           LIBRARY DESTINATION ${LIB_INSTALL_DIR}/strus )


