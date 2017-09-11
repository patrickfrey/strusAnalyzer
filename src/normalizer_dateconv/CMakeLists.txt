cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

# --------------------------------------
# SOURCES AND INCLUDES
# --------------------------------------
set( source_files
	normalizerDateConv.cpp   
)

include_directories(  
	"${PROJECT_SOURCE_DIR}/include" 
	"${PROJECT_SOURCE_DIR}/3rdParty/textwolf/include"
	"${Intl_INCLUDE_DIRS}"
	"${Boost_INCLUDE_DIRS}"
	"${strusbase_INCLUDE_DIRS}"
)

link_directories(
	"${CMAKE_CURRENT_BINARY_DIR}/../utils"
	"${Boost_LIBRARY_DIRS}"
)

# -------------------------------------------
# LIBRARY
# -------------------------------------------
add_library( strus_normalizer_dateconv SHARED ${source_files} libstrus_normalizer_dateconv.cpp )
target_link_libraries( strus_normalizer_dateconv strusanalyzer_private_utils ${Boost_LIBRARIES})

set_target_properties(
    strus_normalizer_dateconv
    PROPERTIES
    DEBUG_POSTFIX "_d"
    SOVERSION "${STRUS_MAJOR_VERSION}.${STRUS_MINOR_VERSION}"
    VERSION ${STRUS_VERSION}
)


# ------------------------------
# INSTALLATION
# ------------------------------
install( TARGETS strus_normalizer_dateconv
           LIBRARY DESTINATION ${LIB_INSTALL_DIR}/strus )


