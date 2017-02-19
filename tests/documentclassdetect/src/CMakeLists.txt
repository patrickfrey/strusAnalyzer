cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

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
	"${Boost_LIBRARY_DIRS}"
	"${strusbase_LIBRARY_DIRS}"
)

add_executable( testDocumentClassDetect testDocumentClassDetect.cpp)
target_link_libraries( testDocumentClassDetect strus_error strus_textproc strusanalyzer_private_utils strus_base ${Boost_LIBRARIES} ${Intl_LIBRARIES} )


