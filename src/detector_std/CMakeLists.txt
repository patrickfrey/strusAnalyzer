cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

# --------------------------------------
# SOURCES AND INCLUDES
# --------------------------------------
set(source_files
	standardDocumentClassDetector.cpp
	libstrus_detector_std.cpp
)

include_directories(  
	"${Intl_INCLUDE_DIRS}"
	"${Boost_INCLUDE_DIRS}"
	"${PROJECT_SOURCE_DIR}/include" 
	"${strusbase_INCLUDE_DIRS}"
)

link_directories(
	"${CMAKE_CURRENT_BINARY_DIR}/../utils"
	"${Boost_LIBRARY_DIRS}"
)

# -------------------------------------------
# LIBRARY
# -------------------------------------------
add_library( strus_detector_std SHARED ${source_files} )
target_link_libraries( strus_detector_std strusanalyzer_private_utils )
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

