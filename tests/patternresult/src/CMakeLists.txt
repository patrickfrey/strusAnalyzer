cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

include_directories(
	${Boost_INCLUDE_DIRS}
	"${Intl_INCLUDE_DIRS}"
	"${PROJECT_SOURCE_DIR}/include"
	"${strusbase_INCLUDE_DIRS}"
)

link_directories(
	"${MAIN_LIBRARY_DIR}/pattern" 
	${Boost_LIBRARY_DIRS}
	"${strusbase_LIBRARY_DIRS}"
)

add_cppcheck( testPatternResultFormat testPatternResultFormat.cpp )

add_executable( testPatternResultFormat testPatternResultFormat.cpp)
target_link_libraries( testPatternResultFormat strus_pattern_resultformat strus_error strusanalyzer_private_utils ${Boost_LIBRARIES} ${Intl_LIBRARIES} )


