cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

include_directories(
	${Boost_INCLUDE_DIRS}
	"${Intl_INCLUDE_DIRS}"
	"${PROJECT_SOURCE_DIR}/include"
	"${strusbase_INCLUDE_DIRS}"
)

link_directories(
	"${MAIN_LIBRARY_DIR}/segmenter_cjson"
	${Boost_LIBRARY_DIRS}
	"${strusbase_LIBRARY_DIRS}"
)

add_cppcheck( testSplitJsonLs testSplitJsonLs.cpp )

add_executable( testSplitJsonLs testSplitJsonLs.cpp)
target_link_libraries( testSplitJsonLs strus_segmenter_cjson strus_segmenter_utils strus_base  strus_error strusanalyzer_private_utils ${Boost_LIBRARIES} ${Intl_LIBRARIES} )
