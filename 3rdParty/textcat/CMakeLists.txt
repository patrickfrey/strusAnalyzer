cmake_minimum_required(VERSION 2.6 FATAL_ERROR)

# --------------------------------------
# SOURCES AND INCLUDES
# --------------------------------------
set(textcat_source_files
	src/common.c
	src/fingerprint.c
	src/testtextcat.c
	src/textcat.c
	src/wg_mempool.c
)

add_definitions( "-D_XOPEN_SOURCE=500" )

# -------------------------------------------
# SNOWBALL STEMMER LIBRARY
# -------------------------------------------
add_library( strus_textcat SHARED ${textcat_source_files})
set_target_properties(
    strus_textcat
    PROPERTIES
    DEBUG_POSTFIX "_d"
    SOVERSION "${STRUS_MAJOR_VERSION}.${STRUS_MINOR_VERSION}"
    VERSION ${STRUS_VERSION}
)

# ------------------------------
# PROGRAMS
# ------------------------------

add_executable( createfp src/createfp.c )
target_link_libraries( createfp strus_textcat )

add_executable( textcatcli src/textcatcli.cpp )
target_link_libraries( textcatcli strus_textcat )

# ------------------------------
# INSTALLATION
# ------------------------------
install( TARGETS strus_textcat
           LIBRARY DESTINATION ${LIB_INSTALL_DIR}/strus )
