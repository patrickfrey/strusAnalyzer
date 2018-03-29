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
	"${MAIN_LIBRARY_DIR}/prgload_std"
	"${MAIN_LIBRARY_DIR}/segmenter_textwolf"
	${Boost_LIBRARY_DIRS}
	"${strusbase_LIBRARY_DIRS}"
)

add_cppcheck( testDocumentAnalyzerLoadProgram testDocumentAnalyzerLoadProgram.cpp )

add_executable( testDocumentAnalyzerLoadProgram testDocumentAnalyzerLoadProgram.cpp)
target_link_libraries( testDocumentAnalyzerLoadProgram strus_analyzer strus_analyzer_prgload_std strus_segmenter_textwolf strus_pattern_test strus_error strus_textproc strus_base   ${Boost_LIBRARIES} ${Intl_LIBRARIES} )


