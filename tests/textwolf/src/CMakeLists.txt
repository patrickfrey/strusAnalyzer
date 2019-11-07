cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

include_directories(
	${Boost_INCLUDE_DIRS}
	"${Intl_INCLUDE_DIRS}"
	"${PROJECT_SOURCE_DIR}/include"
	"${strusbase_INCLUDE_DIRS}"
	"${TEXTWOLF_INCLUDE_DIRS}"
)

link_directories(
	${Boost_LIBRARY_DIRS}
	"${strusbase_LIBRARY_DIRS}"
)

add_cppcheck( testEventsTextwolf testEventsTextwolf.cpp )

add_executable( testEventsTextwolf testEventsTextwolf.cpp)
target_link_libraries( testEventsTextwolf strus_segmenter_utils strus_base  strus_error  ${Boost_LIBRARIES} ${Intl_LIBRARIES} )


