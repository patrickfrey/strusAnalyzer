cmake_minimum_required(VERSION 2.6 FATAL_ERROR)

# --------------------------------------
# SOURCES AND INCLUDES
# --------------------------------------
set(source_files
	libstrus_aggregator_vsm.cpp
)

include_directories(  
	"${PROJECT_SOURCE_DIR}/include" 
)


# -------------------------------------------
# LIBRARY
# -------------------------------------------
add_library( strus_aggregator_vsm SHARED ${source_files} )
set_target_properties(
    strus_aggregator_vsm
    PROPERTIES
    DEBUG_POSTFIX "_d"
    SOVERSION "${STRUS_MAJOR_VERSION}.${STRUS_MINOR_VERSION}"
    VERSION ${STRUS_VERSION}
)


# ------------------------------
# INSTALLATION
# ------------------------------
install( TARGETS strus_aggregator_vsm
           LIBRARY DESTINATION ${LIB_INSTALL_DIR}/strus )


