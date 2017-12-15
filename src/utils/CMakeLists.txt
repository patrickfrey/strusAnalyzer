cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

# --------------------------------------
# SOURCES AND INCLUDES
# --------------------------------------
include_directories(
	${Boost_INCLUDE_DIRS}
	"${PROJECT_SOURCE_DIR}/include"
	"${strusbase_INCLUDE_DIRS}"
	"${TEXTWOLF_INCLUDE_DIRS}"
)
link_directories(
	${Boost_LIBRARY_DIRS}
)


# -------------------------------------------
# UTILS LIBRARY
# Library with some helper functions used by strus libraries and programs but not part of the public interface
# -------------------------------------------
add_library( strusanalyzer_private_utils STATIC internationalization.cpp tokenizeHelpers.cpp textEncoder.cpp )
target_link_libraries( strusanalyzer_private_utils  "${Boost_LIBRARIES}" "${Intl_LIBRARIES}" )
set_property( TARGET strusanalyzer_private_utils PROPERTY POSITION_INDEPENDENT_CODE TRUE )
