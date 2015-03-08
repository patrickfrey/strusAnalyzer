cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

# --------------------------------------
# SOURCES AND INCLUDES
# --------------------------------------
set(source_files
   snowball.cpp
   libstrus_normalizer_snowball.cpp  
)

include_directories(  
	"${PROJECT_SOURCE_DIR}/include" 
	"${PROJECT_SOURCE_DIR}/3rdParty/snowball/libstemmer_c/include"
	"${PROJECT_SOURCE_DIR}/3rdParty/textwolf/include"
)

link_directories(
	"${PROJECT_SOURCE_DIR}/3rdParty/snowball/libstemmer_c"
	"${PROJECT_SOURCE_DIR}/src/utils"
)

# -------------------------------------------
# LIBRARY
# -------------------------------------------
add_library( strus_normalizer_snowball SHARED ${source_files} )
target_link_libraries( strus_normalizer_snowball  stemmer strus_private_utils )
set_target_properties(
    strus_normalizer_snowball
    PROPERTIES
    DEBUG_POSTFIX "_d"
    SOVERSION "${STRUS_MAJOR_VERSION}.${STRUS_MINOR_VERSION}"
    VERSION ${STRUS_VERSION}
)


# ------------------------------
# INSTALLATION
# ------------------------------
install( TARGETS strus_normalizer_snowball
           LIBRARY DESTINATION lib/strus )

