cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

include_directories(
	${Boost_INCLUDE_DIRS}
	"${Intl_INCLUDE_DIRS}"
	"${PROJECT_SOURCE_DIR}/include"
	"${strusbase_INCLUDE_DIRS}"
	${TESTUTILS_INCLUDE_DIRS}
)

link_directories(
	"${MAIN_SOURCE_DIR}/analyzer" 
	"${MAIN_SOURCE_DIR}/textproc" 
	"${MAIN_SOURCE_DIR}/utils" 
	${Boost_LIBRARY_DIRS}
	"${strusbase_LIBRARY_DIRS}"
	${TESTUTILS_LIBRARY_DIRS}
)

add_executable( testQueryTree testQueryTree.cpp)
target_link_libraries(
	testQueryTree
	strus_error strus_base strus_analyzer_objbuild  strus_analyzer strus_textproc  strusanalyzer_private_utils 
	${Boost_LIBRARIES} ${Intl_LIBRARIES}
)


