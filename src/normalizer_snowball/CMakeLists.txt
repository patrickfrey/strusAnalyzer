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
	"${SNOWBALL_INCLUDE_DIRS}"
	"${TEXTWOLF_INCLUDE_DIRS}"
	"${Intl_INCLUDE_DIRS}"
	"${Boost_INCLUDE_DIRS}"
	"${strusbase_INCLUDE_DIRS}"
)

link_directories(
	"${SNOWBALL_LIBRARY_DIRS}"
	"${UTILS_LIBRARY_DIRS}"
	"${Boost_LIBRARY_DIRS}"
)

# -------------------------------------------
# LIBRARY
# -------------------------------------------
add_library( strus_normalizer_snowball SHARED ${source_files} )
target_link_libraries( strus_normalizer_snowball  strus_stemmer strusanalyzer_private_utils )
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
           LIBRARY DESTINATION ${LIB_INSTALL_DIR}/strus )

