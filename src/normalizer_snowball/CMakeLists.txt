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
)

# -------------------------------------------
# LIBRARY
# -------------------------------------------
add_library( strus_normalizer_snowball SHARED ${source_files} )
target_link_libraries( strus_normalizer_snowball stemmer)


# ------------------------------
# INSTALLATION
# ------------------------------
install( TARGETS strus_normalizer_snowball
           LIBRARY DESTINATION lib/strus )

