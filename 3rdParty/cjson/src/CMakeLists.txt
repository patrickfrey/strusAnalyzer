cmake_minimum_required(VERSION 2.6 FATAL_ERROR)

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
add_library( cjson STATIC ${source_files})

