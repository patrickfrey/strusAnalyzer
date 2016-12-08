cmake_minimum_required(VERSION 2.6 FATAL_ERROR)

# --------------------------------------
# SOURCES AND INCLUDES
# --------------------------------------
set(source_files
	featureConfig.cpp
	featureConfigMap.cpp
	featureContextMap.cpp
	patternFeatureConfig.cpp
	patternFeatureConfigMap.cpp
	patternMatchConfigMap.cpp
	patternMatchContextMap.cpp
	segmentProcessor.cpp
	documentAnalyzer.cpp
	documentAnalyzerContext.cpp
	queryAnalyzer.cpp
	queryAnalyzerContext.cpp
)

include_directories(  
	"${PROJECT_SOURCE_DIR}/include" 
	"${Intl_INCLUDE_DIRS}"
 	"${Boost_INCLUDE_DIRS}"
	"${strusbase_INCLUDE_DIRS}"
)

link_directories(
 	"${PROJECT_SOURCE_DIR}/src/segmenter_textwolf"
 	"${PROJECT_SOURCE_DIR}/src/textproc"
 	"${PROJECT_SOURCE_DIR}/src/utils"
)


# -------------------------------------------
# LIBRARY
# -------------------------------------------
add_library( strus_analyzer SHARED  libstrus_analyzer.cpp  ${source_files} )
target_link_libraries( strus_analyzer strus_segmenter_textwolf strus_textproc strus_private_utils )
set_target_properties(
    strus_analyzer
    PROPERTIES
    DEBUG_POSTFIX "_d"
    SOVERSION "${STRUS_MAJOR_VERSION}.${STRUS_MINOR_VERSION}"
    VERSION ${STRUS_VERSION}
)

add_library( strus_analyzer_objbuild SHARED libstrus_analyzer_objbuild.cpp )
target_link_libraries( strus_analyzer_objbuild strus_analyzer strus_segmenter_textwolf strus_segmenter_cjson strus_segmenter_tsv strus_textproc strus_private_utils )
set_target_properties(
    strus_analyzer_objbuild
    PROPERTIES
    DEBUG_POSTFIX "_d"
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

