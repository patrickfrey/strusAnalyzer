cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

# --------------------------------------
# SOURCES AND INCLUDES
# --------------------------------------
set(snowball_source_files
  src_c/stem_UTF_8_danish.c
  src_c/stem_UTF_8_dutch.c
  src_c/stem_UTF_8_english.c
  src_c/stem_UTF_8_finnish.c
  src_c/stem_UTF_8_french.c
  src_c/stem_UTF_8_german.c
  src_c/stem_UTF_8_hungarian.c
  src_c/stem_UTF_8_italian.c
  src_c/stem_UTF_8_norwegian.c
  src_c/stem_UTF_8_porter.c
  src_c/stem_UTF_8_portuguese.c
  src_c/stem_UTF_8_romanian.c
  src_c/stem_UTF_8_russian.c
  src_c/stem_UTF_8_spanish.c
  src_c/stem_UTF_8_swedish.c
  src_c/stem_UTF_8_turkish.c
  runtime/api.c
  runtime/utilities.c
  libstemmer/libstemmer.c
)

include_directories( include libstemmer runtime)

# -------------------------------------------
# SNOWBALL STEMMER LIBRARY
# -------------------------------------------
add_library( strus_stemmer SHARED ${snowball_source_files})
set_target_properties(
    strus_stemmer
    PROPERTIES
    DEBUG_POSTFIX "${CMAKE_DEBUG_POSTFIX}"
    SOVERSION "${STRUS_MAJOR_VERSION}.${STRUS_MINOR_VERSION}"
    VERSION ${STRUS_VERSION}
)

# ------------------------------
# INSTALLATION
# ------------------------------
install( TARGETS strus_stemmer
           LIBRARY DESTINATION ${LIB_INSTALL_DIR}/strus )
