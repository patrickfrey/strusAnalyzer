cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

include_directories(
	${Boost_INCLUDE_DIRS}
	"${Intl_INCLUDE_DIRS}"
	"${PROJECT_SOURCE_DIR}/include"
	"${strusbase_INCLUDE_DIRS}"
)

link_directories(
	"${MAIN_LIBRARY_DIR}/analyzer"
	"${MAIN_LIBRARY_DIR}/textproc"
	"${MAIN_LIBRARY_DIR}/utils"
	"${MAIN_LIBRARY_DIR}/normalizer_substrindex"
	${Boost_LIBRARY_DIRS}
	"${strusbase_LIBRARY_DIRS}"
)

add_cppcheck( testNormalizerSubStringIndex testNormalizerSubStringIndex.cpp )

add_executable( testNormalizerSubStringIndex testNormalizerSubStringIndex.cpp)
target_link_libraries( testNormalizerSubStringIndex strus_error strus_textproc ${Boost_LIBRARIES} ${Intl_LIBRARIES} )


