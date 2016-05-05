cmake_minimum_required(VERSION 2.6 FATAL_ERROR)

include_directories(
	"${Boost_INCLUDE_DIRS}"
	"${Intl_INCLUDE_DIRS}"
	"${PROJECT_SOURCE_DIR}/include"
	"${strusbase_INCLUDE_DIRS}"
)

link_directories(
	"${PROJECT_SOURCE_DIR}/src/analyzer"
	"${PROJECT_SOURCE_DIR}/src/textproc"
	"${PROJECT_SOURCE_DIR}/src/utils"
	"${PROJECT_SOURCE_DIR}/src/detector_std"
	"${PROJECT_SOURCE_DIR}/src/tokenizer_textcat"
	"${Boost_LIBRARY_DIRS}"
	"${strusbase_LIBRARY_DIRS}"
)

add_executable( testTextcat testTextcat.cpp)
target_link_libraries( testTextcat strus_error strus_tokenizer_textcat strus_textproc strus_private_utils ${Boost_LIBRARIES} ${Intl_LIBRARIES} )


