cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

# --------------------------------------
# SOURCES AND INCLUDES
# --------------------------------------
set(source_files
	featureConfig.cpp
	featureConfigMap.cpp
	patternFeatureConfig.cpp
	patternFeatureConfigMap.cpp
	patternMatchConfigMap.cpp
	patternMatchContextMap.cpp
	segmentProcessor.cpp
	documentAnalyzerInstance.cpp
	documentAnalyzerContext.cpp
	documentAnalyzerMap.cpp
	queryAnalyzerInstance.cpp
	queryAnalyzerContext.cpp
)

include_directories(  
	"${PROJECT_SOURCE_DIR}/include" 
	"${Intl_INCLUDE_DIRS}"
 	${Boost_INCLUDE_DIRS}
	"${strusbase_INCLUDE_DIRS}"
)

link_directories(
 	"${MAIN_SOURCE_DIR}/segmenter_textwolf"
 	"${MAIN_SOURCE_DIR}/textproc"
 	"${UTILS_LIBRARY_DIRS}"
	${Boost_LIBRARY_DIRS}
	"${strusbase_LIBRARY_DIRS}"
)


# -------------------------------------------
# LIBRARY
# -------------------------------------------
add_cppcheck( strus_analyzer libstrus_analyzer.cpp  ${source_files} )

add_library( strus_analyzer SHARED  libstrus_analyzer.cpp  ${source_files} )
target_link_libraries( strus_analyzer strus_segmenter_textwolf strus_textproc strus_pattern_resultformat strusanalyzer_private_utils strus_base )
set_target_properties(
    strus_analyzer
    PROPERTIES
    DEBUG_POSTFIX "${CMAKE_DEBUG_POSTFIX}"
    SOVERSION "${STRUS_MAJOR_VERSION}.${STRUS_MINOR_VERSION}"
    VERSION ${STRUS_VERSION}
)

add_library( strus_analyzer_objbuild SHARED libstrus_analyzer_objbuild.cpp )
target_link_libraries( strus_analyzer_objbuild strus_analyzer strus_segmenter_textwolf strus_segmenter_cjson strus_segmenter_tsv strus_textproc strus_contentstats_std strus_detector_std strusanalyzer_private_utils )
set_target_properties(
    strus_analyzer_objbuild
    PROPERTIES
    DEBUG_POSTFIX "${CMAKE_DEBUG_POSTFIX}"
    SOVERSION "${STRUS_MAJOR_VERSION}.${STRUS_MINOR_VERSION}"
    VERSION ${STRUS_VERSION}
)

# ------------------------------
# INSTALLATION
# ------------------------------
install( TARGETS strus_analyzer
           LIBRARY DESTINATION ${LIB_INSTALL_DIR}/strus )

install( TARGETS strus_analyzer_objbuild
           LIBRARY DESTINATION ${LIB_INSTALL_DIR}/strus )

