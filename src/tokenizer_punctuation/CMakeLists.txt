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


# -------------------------------------------
# LIBRARY
# -------------------------------------------
add_library( strus_tokenizer_punctuation SHARED ${source_files} )


# ------------------------------
# INSTALLATION
# ------------------------------
install( TARGETS strus_tokenizer_punctuation
           LIBRARY DESTINATION lib/strus )


