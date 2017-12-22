cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

# must be done BEFORE project in order to work!
include(cmake/install_prefix.cmake)

# Project globals:
project( StrusAnalyzer )
set( STRUS_MAJOR_VERSION 0 )
set( STRUS_MINOR_VERSION 15 )
set( STRUS_PATCH_VERSION 4 )
set( STRUS_VERSION ${STRUS_MAJOR_VERSION}.${STRUS_MINOR_VERSION}.${STRUS_PATCH_VERSION} )

if(NOT STRUS_ALL)
include( cmake/dist.cmake )
include( cmake/LinuxDistribution.cmake )
include( cmake/build_rules.cmake )
include( cmake/boost.cmake)
include( cmake/intl.cmake )
include( cmake/find_strus_package.cmake )
include( cmake/link_rules.cmake )
endif()

find_strus_package( base )
set( CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" )
configure_file( "${PROJECT_SOURCE_DIR}/include/strus/versionAnalyzer.hpp.in"  "${CMAKE_BINARY_DIR}/include/strus/versionAnalyzer.hpp"  @ONLY )
configure_file( "${PROJECT_SOURCE_DIR}/dist/obs/deploy_to_obs.sh.in"  "${PROJECT_SOURCE_DIR}/dist/obs/deploy_to_obs.sh"  @ONLY )
configure_file( "${PROJECT_SOURCE_DIR}/dist/archlinux/PKGBUILD.in"  "${PROJECT_SOURCE_DIR}/dist/archlinux/PKGBUILD"  @ONLY )
configure_file( "${PROJECT_SOURCE_DIR}/dist/obs/strusanalyzer.dsc.in"  "${PROJECT_SOURCE_DIR}/dist/obs/strusanalyzer.dsc"  @ONLY )
configure_file( "${PROJECT_SOURCE_DIR}/dist/redhat/strusanalyzer.spec.in"  "${PROJECT_SOURCE_DIR}/dist/redhat/strusanalyzer.spec"  @ONLY )

set( STRUS_GETTEXT_PACKAGE "strus-dom" )	#... 1st parameter of bindtextdomain(...)
set( STRUS_GETTEXT_LOCALEDIR "" )			#... 2nd parameter of bindtextdomain(...)
configure_file( "${PROJECT_SOURCE_DIR}/src/utils/internationalization.cpp.in"  "${PROJECT_SOURCE_DIR}/src/utils/internationalization.cpp"  @ONLY )

IF (WIN32)
set( STRUS_RESOURCE_PATHSEP ';')
set( STRUS_RESOURCE_DIRSEP '\\' )
ELSE (WIN32)
set( STRUS_RESOURCE_PATHSEP ':')
set( STRUS_RESOURCE_DIRSEP '/' )
ENDIF (WIN32)
configure_file( "${PROJECT_SOURCE_DIR}/src/textproc/resourceDirectory.hpp.in"  "${PROJECT_SOURCE_DIR}/src/textproc/resourceDirectory.hpp"  @ONLY )

enable_testing()

# Path declarations:
set( TEXTCAT_INCLUDE_DIRS           "${CMAKE_CURRENT_BINARY_DIR}/3rdParty/textcat/src" "${PROJECT_SOURCE_DIR}/3rdParty/textcat/src" )
set( TEXTCAT_LIBRARY_DIRS           "${CMAKE_CURRENT_BINARY_DIR}/3rdParty/textcat" )
set( TEXTWOLF_INCLUDE_DIRS       "${CMAKE_CURRENT_BINARY_DIR}/3rdParty/textwolf/include"  "${PROJECT_SOURCE_DIR}/3rdParty/textwolf/include" )
set( CJSON_INCLUDE_DIRS              "${CMAKE_CURRENT_BINARY_DIR}/3rdParty/cjson/include"  "${PROJECT_SOURCE_DIR}/3rdParty/cjson/include" )
set( CJSON_LIBRARY_DIRS               "${CMAKE_CURRENT_BINARY_DIR}/3rdParty/cjson" )
set( SNOWBALL_INCLUDE_DIRS       "${CMAKE_CURRENT_BINARY_DIR}/3rdParty/snowball/libstemmer_c/include"   "${PROJECT_SOURCE_DIR}/3rdParty/snowball/libstemmer_c/include" )
set( SNOWBALL_LIBRARY_DIRS        "${CMAKE_CURRENT_BINARY_DIR}/3rdParty/snowball/libstemmer_c" )
set( CNODETRIE_INCLUDE_DIRS       "${CMAKE_CURRENT_BINARY_DIR}/3rdParty/compactNodeTrie/include"  "${PROJECT_SOURCE_DIR}/3rdParty/compactNodeTrie/include" )
set( CNODETRIE_LIBRARY_DIRS       "${CMAKE_CURRENT_BINARY_DIR}/3rdParty/compactNodeTrie/src" )
set( UTILS_LIBRARY_DIRS                 "${CMAKE_CURRENT_BINARY_DIR}/src/utils" )
set( MAIN_SOURCE_DIR                    "${PROJECT_SOURCE_DIR}/src" )

# Subdirectories:
add_subdirectory( 3rdParty )
add_subdirectory( src )
add_subdirectory( tests )
  
if(NOT STRUS_ALL)
include( cmake/report_build_settings.cmake )
endif()

# Install header files:
install(DIRECTORY include/strus
          DESTINATION include
          FILES_MATCHING PATTERN "*.hpp")
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/include/strus
          DESTINATION include
          FILES_MATCHING PATTERN "versionAnalyzer.hpp")
if(NOT STRUS_ALL)
include( cmake/standard_targets.cmake )
endif()
