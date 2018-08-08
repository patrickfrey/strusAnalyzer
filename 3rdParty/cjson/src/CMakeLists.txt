cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

# --------------------------------------
# SOURCES AND INCLUDES
# --------------------------------------
set(source_files
     cJSON.c
)

include_directories( "${PROJECT_SOURCE_DIR}/3rdParty/cjson/include/cjson" )

# -------------------------------------------
# SNOWBALL STEMMER LIBRARY
# -------------------------------------------
add_library( strus_cjson STATIC ${source_files})
set_property( TARGET strus_cjson PROPERTY POSITION_INDEPENDENT_CODE TRUE )

