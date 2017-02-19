cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

# --------------------------------------
# SOURCES AND INCLUDES
# --------------------------------------
set( source_files
  compactNodeTrie.cpp
)

include_directories(
	"${PROJECT_SOURCE_DIR}/include"
	"${Intl_INCLUDE_DIRS}"
	"${Boost_INCLUDE_DIRS}"
	"${PROJECT_SOURCE_DIR}/3rdParty/compactNodeTrie/include"
)

# -------------------------------------------
# COMPACT NODE TRIE LIBRARY
# -------------------------------------------
add_library( compactnodetrie_analyzer_static STATIC ${source_files})



