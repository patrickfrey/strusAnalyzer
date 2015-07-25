cmake_minimum_required(VERSION 2.6 FATAL_ERROR)

# --------------------------------------
# SOURCES AND INCLUDES
# --------------------------------------
set(source_files
	standardDocumentClassDetector.cpp
	libstrus_detector_std.cpp
)

include_directories(  
	"${PROJECT_SOURCE_DIR}/include" 
)
link_directories(
)

# -------------------------------------------
# LIBRARY
# -------------------------------------------
add_library( strus_detector_std SHARED ${source_files} )
set_target_properties(
    strus_detector_std
    PROPERTIES
    DEBUG_POSTFIX "_d"
    SOVERSION "${STRUS_MAJOR_VERSION}.${STRUS_MINOR_VERSION}"
    VERSION ${STRUS_VERSION}
)

# ------------------------------
# INSTALLATION
# ------------------------------
install( TARGETS strus_detector_std
           LIBRARY DESTINATION ${LIB_INSTALL_DIR}/strus )

