cmake_minimum_required(VERSION 2.6 FATAL_ERROR)

# --------------------------------------
# SOURCES AND INCLUDES
# --------------------------------------
set(snowball_source_files
  src_c/stem_ISO_8859_1_danish.c
  src_c/stem_UTF_8_danish.c
  src_c/stem_ISO_8859_1_dutch.c
  src_c/stem_UTF_8_dutch.c
  src_c/stem_ISO_8859_1_english.c
  src_c/stem_UTF_8_english.c
  src_c/stem_ISO_8859_1_finnish.c
  src_c/stem_UTF_8_finnish.c
  src_c/stem_ISO_8859_1_french.c
  src_c/stem_UTF_8_french.c
  src_c/stem_ISO_8859_1_german.c
  src_c/stem_UTF_8_german.c
  src_c/stem_ISO_8859_2_hungarian.c
  src_c/stem_UTF_8_hungarian.c
  src_c/stem_ISO_8859_1_italian.c
  src_c/stem_UTF_8_italian.c
  src_c/stem_ISO_8859_1_norwegian.c
  src_c/stem_UTF_8_norwegian.c
  src_c/stem_ISO_8859_1_porter.c
  src_c/stem_UTF_8_porter.c
  src_c/stem_ISO_8859_1_portuguese.c
  src_c/stem_UTF_8_portuguese.c
  src_c/stem_ISO_8859_2_romanian.c
  src_c/stem_UTF_8_romanian.c
  src_c/stem_KOI8_R_russian.c
  src_c/stem_UTF_8_russian.c
  src_c/stem_ISO_8859_1_spanish.c
  src_c/stem_UTF_8_spanish.c
  src_c/stem_ISO_8859_1_swedish.c
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
add_library( stemmer SHARED ${snowball_source_files})
set_target_properties(
    stemmer
    PROPERTIES
    DEBUG_POSTFIX "_d"
)
# ------------------------------
# INSTALLATION
# ------------------------------
install( TARGETS stemmer
           LIBRARY DESTINATION ${LIB_INSTALL_DIR}/strus )

