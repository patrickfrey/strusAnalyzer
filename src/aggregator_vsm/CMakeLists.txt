cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

# --------------------------------------
# SOURCES AND INCLUDES
# --------------------------------------
set(source_files
	libstrus_aggregator_vsm.cpp
)

include_directories(  
	"${Intl_INCLUDE_DIRS}"
	${Boost_INCLUDE_DIRS}
	"${PROJECT_SOURCE_DIR}/include"
	"${strusbase_INCLUDE_DIRS}"
)

link_directories(
	"${strusbase_LIBRARY_DIRS}"
	"${UTILS_LIBRARY_DIRS}"
	${Boost_LIBRARY_DIRS}
)

# -------------------------------------------
# LIBRARY
# -------------------------------------------
add_cppcheck( strus_aggregator_vsm ${source_files} )

add_library( strus_aggregator_vsm SHARED ${source_files} )
target_link_libraries( strus_aggregator_vsm strusanalyzer_private_utils strus_base )
set_target_properties(
    strus_aggregator_vsm
    PROPERTIES
    DEBUG_POSTFIX "${CMAKE_DEBUG_POSTFIX}"
    SOVERSION "${STRUS_MAJOR_VERSION}.${STRUS_MINOR_VERSION}"
    VERSION ${STRUS_VERSION}
)


# ------------------------------
# INSTALLATION
# ------------------------------
install( TARGETS strus_aggregator_vsm
           LIBRARY DESTINATION ${LIB_INSTALL_DIR}/strus )


