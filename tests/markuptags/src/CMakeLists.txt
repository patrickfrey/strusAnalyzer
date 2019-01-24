cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

include_directories(
	${Boost_INCLUDE_DIRS}
	"${Intl_INCLUDE_DIRS}"
	"${PROJECT_SOURCE_DIR}/tests/utils"
	"${PROJECT_SOURCE_DIR}/include"
	"${strusbase_INCLUDE_DIRS}"
	${TEXTWOLF_INCLUDE_DIRS}
)

link_directories(
	"${MAIN_LIBRARY_DIR}/segmenter_textwolf"
	${Boost_LIBRARY_DIRS}
	"${strusbase_LIBRARY_DIRS}"
)

add_cppcheck( testMarkupTags  testMarkupTags.cpp )

add_executable( testMarkupTags testMarkupTags.cpp)
target_link_libraries( testMarkupTags  strus_markup_document_tags strus_segmenter_utils strus_detector_std  strus_filelocator strus_base  strus_error strus_textproc strusanalyzer_private_utils ${Boost_LIBRARIES} ${Intl_LIBRARIES} )

