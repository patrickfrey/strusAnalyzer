cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

# --------------------------------------
# SOURCES AND INCLUDES
# --------------------------------------
set( source_files
	normalizerCharConv.cpp   
)

include_directories(  
	"${PROJECT_SOURCE_DIR}/include" 
	"${PROJECT_SOURCE_DIR}/3rdParty/textwolf/include"
)

link_directories(
	"${PROJECT_SOURCE_DIR}/src/utils"
)

# -------------------------------------------
# LIBRARY
# -------------------------------------------
add_library( strus_normalizer_charconv SHARED ${source_files} libstrus_normalizer_charconv.cpp )
target_link_libraries( strus_normalizer_charconv strus_private_utils )

set_target_properties(
    strus_normalizer_charconv
    PROPERTIES
    DEBUG_POSTFIX "_d"
    SOVERSION "${STRUS_MAJOR_VERSION}.${STRUS_MINOR_VERSION}"
    VERSION ${STRUS_VERSION}
)


# ------------------------------
# INSTALLATION
# ------------------------------
install( TARGETS strus_normalizer_charconv
           LIBRARY DESTINATION lib/strus )


