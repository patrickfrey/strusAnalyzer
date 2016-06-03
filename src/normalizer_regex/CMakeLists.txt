cmake_minimum_required(VERSION 2.6 FATAL_ERROR)

# --------------------------------------
# SOURCES AND INCLUDES
# --------------------------------------
set( source_files
	normalizerRegex.cpp   
)

include_directories(  
	"${PROJECT_SOURCE_DIR}/include" 
	"${PROJECT_SOURCE_DIR}/3rdParty/textwolf/include"
	"${Intl_INCLUDE_DIRS}"
	"${Boost_INCLUDE_DIRS}"
	"${strusbase_INCLUDE_DIRS}"
)

link_directories(
	"${PROJECT_SOURCE_DIR}/src/utils"
)

# -------------------------------------------
# LIBRARY
# -------------------------------------------
add_library( strus_normalizer_regex SHARED ${source_files} libstrus_normalizer_regex.cpp )
target_link_libraries( strus_normalizer_regex strus_private_utils )

set_target_properties(
    strus_normalizer_regex
    PROPERTIES
    DEBUG_POSTFIX "_d"
    SOVERSION "${STRUS_MAJOR_VERSION}.${STRUS_MINOR_VERSION}"
    VERSION ${STRUS_VERSION}
)


# ------------------------------
# INSTALLATION
# ------------------------------
install( TARGETS strus_normalizer_regex
           LIBRARY DESTINATION ${LIB_INSTALL_DIR}/strus )

