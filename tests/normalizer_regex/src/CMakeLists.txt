cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

include_directories(
	${Boost_INCLUDE_DIRS}
	"${Intl_INCLUDE_DIRS}"
	"${PROJECT_SOURCE_DIR}/include"
	"${strusbase_INCLUDE_DIRS}"
)

link_directories(
	"${MAIN_SOURCE_DIR}/analyzer"
	"${MAIN_SOURCE_DIR}/textproc"
	"${MAIN_SOURCE_DIR}/utils"
	"${MAIN_SOURCE_DIR}/normalizer_regex"
	${Boost_LIBRARY_DIRS}
	"${strusbase_LIBRARY_DIRS}"
)

add_executable( testNormalizerRegex testNormalizerRegex.cpp)
target_link_libraries( testNormalizerRegex strus_error strus_textproc ${Boost_LIBRARIES} ${Intl_LIBRARIES} )


