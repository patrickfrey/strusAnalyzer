cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

# --------------------------------------
# SOURCES AND INCLUDES
# --------------------------------------
set( source_files
	normalizerDateConv.cpp   
)

include_directories(  
	"${PROJECT_SOURCE_DIR}/include" 
	${TEXTWOLF_INCLUDE_DIRS}
	"${Intl_INCLUDE_DIRS}"
	${Boost_INCLUDE_DIRS}
	"${strusbase_INCLUDE_DIRS}"
)

link_directories(
	"${UTILS_LIBRARY_DIRS}"
	"${strusbase_LIBRARY_DIRS}"
	${Boost_LIBRARY_DIRS}
)

# -------------------------------------------
# LIBRARY
# -------------------------------------------
add_cppcheck( strus_normalizer_dateconv ${source_files}  libstrus_normalizer_dateconv.cpp )

add_library( strus_normalizer_dateconv SHARED ${source_files} libstrus_normalizer_dateconv.cpp )
target_link_libraries( strus_normalizer_dateconv strusanalyzer_private_utils strus_base ${Boost_LIBRARIES})

set_target_properties(
    strus_normalizer_dateconv
    PROPERTIES
    DEBUG_POSTFIX "${CMAKE_DEBUG_POSTFIX}"
    SOVERSION "${STRUS_MAJOR_VERSION}.${STRUS_MINOR_VERSION}"
    VERSION ${STRUS_VERSION}
)


# ------------------------------
# INSTALLATION
# ------------------------------
install( TARGETS strus_normalizer_dateconv
           LIBRARY DESTINATION ${LIB_INSTALL_DIR}/strus )


