cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

# --------------------------------------
# SOURCES AND INCLUDES
# --------------------------------------
set( source_files_pattern_test
	patternLexer.cpp
	patternMatcher.cpp
)

set( source_files_termfeeder
)

set( source_files_serialize
)

include_directories(  
	"${PROJECT_SOURCE_DIR}/include" 
	"${GEN_INCLUDE_DIR}"
	"${Intl_INCLUDE_DIRS}"
 	${Boost_INCLUDE_DIRS}
	"${strusbase_INCLUDE_DIRS}"
)

link_directories(
	"${strusbase_LIBRARY_DIRS}"
	${Boost_LIBRARY_DIRS}
)


# -------------------------------------------
# LIBRARY
# -------------------------------------------
add_cppcheck( strus_pattern_termfeeder libstrus_pattern_termfeeder.cpp  ${source_files_termfeeder} )

add_library( strus_pattern_termfeeder SHARED  libstrus_pattern_termfeeder.cpp  ${source_files_termfeeder} )
target_link_libraries( strus_pattern_termfeeder  strusanalyzer_private_utils strus_base )
set_target_properties(
    strus_pattern_termfeeder
    PROPERTIES
    DEBUG_POSTFIX "${CMAKE_DEBUG_POSTFIX}"
    SOVERSION "${STRUS_MAJOR_VERSION}.${STRUS_MINOR_VERSION}"
    VERSION ${STRUS_VERSION}
)

add_cppcheck( strus_pattern_serialize libstrus_pattern_serialize.cpp  ${source_files_serialize} )

add_library( strus_pattern_serialize SHARED  libstrus_pattern_serialize.cpp  ${source_files_serialize} )
target_link_libraries( strus_pattern_serialize  strusanalyzer_private_utils strus_base )
set_target_properties(
    strus_pattern_serialize
    PROPERTIES
    DEBUG_POSTFIX "${CMAKE_DEBUG_POSTFIX}"
    SOVERSION "${STRUS_MAJOR_VERSION}.${STRUS_MINOR_VERSION}"
    VERSION ${STRUS_VERSION}
)

add_cppcheck( strus_pattern_test libstrus_pattern_test.cpp  ${source_files_pattern_test} )

add_library( strus_pattern_test SHARED libstrus_pattern_test.cpp ${source_files_pattern_test} )
target_link_libraries( strus_pattern_test  strus_base  strusanalyzer_private_utils  ${Intl_LIBRARIES} )

set_target_properties(
    strus_pattern_test
    PROPERTIES
    DEBUG_POSTFIX "${CMAKE_DEBUG_POSTFIX}"
    SOVERSION "${STRUS_MAJOR_VERSION}.${STRUS_MINOR_VERSION}"
    VERSION ${STRUS_VERSION} )

add_library( modstrus_analyzer_pattern_test  MODULE  modstrus_analyzer_pattern_test.cpp )
set_target_properties( modstrus_analyzer_pattern_test PROPERTIES PREFIX "")
target_link_libraries( modstrus_analyzer_pattern_test strus_module_analyzer strus_pattern_test strusanalyzer_private_utils ${Intl_LIBRARIES} )

# Copy pattern module into a created directory easy relocatable by tests:
add_custom_command( TARGET modstrus_analyzer_pattern_test POST_BUILD
					  COMMAND ${CMAKE_COMMAND} -E make_directory  ${CMAKE_BINARY_DIR}/modules/strus )
add_custom_command( TARGET modstrus_analyzer_pattern_test POST_BUILD
					  COMMAND ${CMAKE_COMMAND} -E copy_if_different  $<TARGET_FILE:modstrus_analyzer_pattern_test>  ${CMAKE_BINARY_DIR}/modules/strus/
					  COMMENT "Copy strus pattern_test module built to ${CMAKE_BINARY_DIR}/modules/strus/" )

# ------------------------------
# INSTALLATION
# ------------------------------
install( TARGETS strus_pattern_termfeeder
           LIBRARY DESTINATION ${LIB_INSTALL_DIR}/strus )
install( TARGETS strus_pattern_serialize
           LIBRARY DESTINATION ${LIB_INSTALL_DIR}/strus )
install( TARGETS strus_pattern_test
           LIBRARY DESTINATION ${LIB_INSTALL_DIR}/strus )
install( TARGETS modstrus_analyzer_pattern_test
           LIBRARY DESTINATION ${LIB_INSTALL_DIR}/strus/modules
	   RUNTIME DESTINATION bin )


