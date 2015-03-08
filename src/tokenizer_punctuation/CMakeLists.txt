cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

# --------------------------------------
# SOURCES AND INCLUDES
# --------------------------------------
set(source_files
	libstrus_tokenizer_punctuation.cpp
	punctuation.cpp
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
add_library( strus_tokenizer_punctuation SHARED ${source_files} )
target_link_libraries( strus_tokenizer_punctuation strus_private_utils )
set_target_properties(
    strus_tokenizer_punctuation
    PROPERTIES
    DEBUG_POSTFIX "_d"
    SOVERSION "${STRUS_MAJOR_VERSION}.${STRUS_MINOR_VERSION}"
    VERSION ${STRUS_VERSION}
)


# ------------------------------
# INSTALLATION
# ------------------------------
install( TARGETS strus_tokenizer_punctuation
           LIBRARY DESTINATION lib/strus )


