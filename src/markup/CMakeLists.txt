cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

# --------------------------------------
# SOURCES AND INCLUDES
# --------------------------------------
set(source_files
	tokenMarkup.cpp
	libstrus_markup_std.cpp
)

include_directories(  
	"${Intl_INCLUDE_DIRS}"
	"${Boost_INCLUDE_DIRS}"
	"${PROJECT_SOURCE_DIR}/include" 
	"${strusbase_INCLUDE_DIRS}"
)

link_directories(
	"${UTILS_LIBRARY_DIRS}"
	"${Boost_LIBRARY_DIRS}"
)

# -------------------------------------------
# LIBRARY
# -------------------------------------------
add_library( strus_markup_std SHARED ${source_files} )
target_link_libraries( strus_markup_std strusanalyzer_private_utils )
set_target_properties(
    strus_markup_std
    PROPERTIES
    DEBUG_POSTFIX "${CMAKE_DEBUG_POSTFIX}"
    SOVERSION "${STRUS_MAJOR_VERSION}.${STRUS_MINOR_VERSION}"
    VERSION ${STRUS_VERSION}
)

# ------------------------------
# INSTALLATION
# ------------------------------
install( TARGETS strus_markup_std
           LIBRARY DESTINATION ${LIB_INSTALL_DIR}/strus )

