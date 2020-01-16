cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

include_directories(
	${Boost_INCLUDE_DIRS}
	"${Intl_INCLUDE_DIRS}"
	"${PROJECT_SOURCE_DIR}/include"
	"${strusbase_INCLUDE_DIRS}"
)

link_directories(
	"${MAIN_LIBRARY_DIR}/doctree"
	"${MAIN_LIBRARY_DIR}/textproc"
	${Boost_LIBRARY_DIRS}
	"${strusbase_LIBRARY_DIRS}"
)

add_cppcheck( testDocTree testDocTree.cpp )

add_executable( testDocTree testDocTree.cpp)
target_link_libraries( testDocTree  strus_doctree  strus_base  strus_error strus_textproc strus_detector_std  strus_filelocator ${Boost_LIBRARIES} ${Intl_LIBRARIES} )

