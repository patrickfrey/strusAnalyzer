cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

include_directories(
	"${Boost_INCLUDE_DIRS}"
	"${Intl_INCLUDE_DIRS}"
	"${PROJECT_SOURCE_DIR}/include"
	"${strusbase_INCLUDE_DIRS}"
)

link_directories(
	"${MAIN_SOURCE_DIR}/segmenter_textwolf"
	"${Boost_LIBRARY_DIRS}"
	"${strusbase_LIBRARY_DIRS}"
)

add_executable( testSegmenterTextwolf testSegmenterTextwolf.cpp)
target_link_libraries( testSegmenterTextwolf strus_segmenter_textwolf strus_segmenter_utils strus_detector_std  strus_base  strus_error strusanalyzer_private_utils ${Boost_LIBRARIES} ${Intl_LIBRARIES} )

add_executable( testMarkupTextwolf testMarkupTextwolf.cpp)
target_link_libraries( testMarkupTextwolf strus_segmenter_textwolf strus_segmenter_utils strus_detector_std  strus_base  strus_error strusanalyzer_private_utils ${Boost_LIBRARIES} ${Intl_LIBRARIES} )

