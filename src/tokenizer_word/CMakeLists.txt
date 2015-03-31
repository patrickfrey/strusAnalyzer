cmake_minimum_required(VERSION 2.6 FATAL_ERROR)

# --------------------------------------
# SOURCES AND INCLUDES
# --------------------------------------
set(source_files
	libstrus_tokenizer_word.cpp
)

include_directories(  
	"${PROJECT_SOURCE_DIR}/include" 
	"${PROJECT_SOURCE_DIR}/3rdParty/textwolf/include"
)


# -------------------------------------------
# LIBRARY
# -------------------------------------------
add_library( strus_tokenizer_word SHARED ${source_files} )
set_target_properties(
    strus_tokenizer_word
    PROPERTIES
    DEBUG_POSTFIX "_d"
    SOVERSION "${STRUS_MAJOR_VERSION}.${STRUS_MINOR_VERSION}"
    VERSION ${STRUS_VERSION}
)


# ------------------------------
# INSTALLATION
# ------------------------------
install( TARGETS strus_tokenizer_word
           LIBRARY DESTINATION ${LIB_INSTALL_DIR}/strus )


