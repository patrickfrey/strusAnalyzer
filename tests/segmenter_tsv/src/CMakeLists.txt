cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

include_directories(
	"${Boost_INCLUDE_DIRS}"
	"${Intl_INCLUDE_DIRS}"
	"${PROJECT_SOURCE_DIR}/include"
	"${strusbase_INCLUDE_DIRS}"
)

link_directories(
	"${MAIN_SOURCE_DIR}/segmenter_tsv"
	"${Boost_LIBRARY_DIRS}"
	"${strusbase_LIBRARY_DIRS}"
)

add_executable( testSegmenterTsv testSegmenterTsv.cpp)
target_link_libraries( testSegmenterTsv strus_segmenter_tsv strus_segmenter_utils strus_detector_std  strus_base  strus_error strusanalyzer_private_utils ${Boost_LIBRARIES} ${Intl_LIBRARIES} )
