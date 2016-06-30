cmake_minimum_required(VERSION 2.6 FATAL_ERROR)

# --------------------------------------
# SOURCES AND INCLUDES
# --------------------------------------
set(textcat_source_files
	src/common.c
	src/fingerprint.c
	src/testtextcat.c
	src/textcat.c
	src/wg_mempool.c
)

include_directories( ${CMAKE_CURRENT_BINARY_DIR}/src )

# -------------------------------------------
# CONFIGURATION
# -------------------------------------------

# copied from build_rules.cmake, removed C99 standard (because of compilation issues)
if(CMAKE_COMPILER_IS_GNUCXX)
set( STRUS_OPTIMIZATION_LEVEL "3" )
set( CMAKE_CXX_FLAGS "-std=c++98  -Wall -pedantic -g -Wfatal-errors -fvisibility=hidden -fPIC -O${STRUS_OPTIMIZATION_LEVEL}" )
set( CMAKE_C_FLAGS "-Wall -pedantic -Wfatal-errors -fPIC -O${STRUS_OPTIMIZATION_LEVEL}" )
endif()
if(CMAKE_CXX_COMPILER MATCHES ".*clang")
set( STRUS_OPTIMIZATION_LEVEL "3" )
set( CMAKE_CXX_FLAGS "-std=c++98  -Wall -pedantic -g -Wfatal-errors -fvisibility=hidden -Wno-unused-private-field -fPIC -O${STRUS_OPTIMIZATION_LEVEL}" )
set( CMAKE_C_FLAGS "-Wall -pedantic -Wfatal-errors -fPIC -O${STRUS_OPTIMIZATION_LEVEL}" )
endif()

set( PACKAGE_VERSION "2.2" )

include( CheckIncludeFiles )
check_include_files( "inttypes.h" HAVE_INTTYPES_H )
check_include_files( "limits.h" HAVE_LIMITS_H )
check_include_files( "stdlib.h" HAVE_STDLIB_H )
check_include_files( "string.h" HAVE_STRING_H )
check_include_files( "strings.h" HAVE_STRINGS_H )
check_include_files( "sys/time.h" HAVE_SYS_TIME_H )
check_include_files( "alloca.h" HAVE_ALLOCA_H )
check_include_files( "dlfcn.h" HAVE_DLFCN_H )
check_include_files( "memory.h" HAVE_MEMORY_H )
check_include_files( "stdint.h" HAVE_STDINT_H )

include( CheckFunctionExists )
check_function_exists( "gettimeofday" HAVE_GETTIMEOFDAY )
check_function_exists( "memset" HAVE_MEMSET )
check_function_exists( "strchr" HAVE_STRCHR )
check_function_exists( "strdup" HAVE_STRDUP )
check_function_exists( "strpbrk" HAVE_STRPBRK )
check_function_exists( "malloc" HAVE_MALLOC )
check_function_exists( "realloc" HAVE_REALLOC )
check_function_exists( "vprintf" HAVE_VPRINTF )

include( ac_header_stdc )

configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/src/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/src/config.h )

# -------------------------------------------
# TEXTCAT LANGUAGE DETECTION LIBRARY
# -------------------------------------------
add_library( strus_textcat SHARED ${textcat_source_files})
set_target_properties(
    strus_textcat
    PROPERTIES
    DEBUG_POSTFIX "_d"
    SOVERSION "${STRUS_MAJOR_VERSION}.${STRUS_MINOR_VERSION}"
    VERSION ${STRUS_VERSION}
)

# ------------------------------
# PROGRAMS
# ------------------------------

add_executable( createfp src/createfp.c )
target_link_libraries( createfp strus_textcat )

add_executable( textcatcli src/textcatcli.cpp )
target_link_libraries( textcatcli strus_textcat )

# ------------------------------
# INSTALLATION
# ------------------------------
install( TARGETS strus_textcat
           LIBRARY DESTINATION ${LIB_INSTALL_DIR}/strus )

# ------------------------------
# INSTALLATION
# ------------------------------
install(DIRECTORY DESTINATION share/strus)
install( DIRECTORY langclass/LM
           DESTINATION share/strus/textcat
           FILES_MATCHING PATTERN "*.lm")
install( FILES langclass/conf.txt
           DESTINATION share/strus/textcat)
