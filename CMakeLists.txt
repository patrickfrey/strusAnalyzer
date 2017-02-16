cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

# must be done BEFORE project in order to work!
include(cmake/install_prefix.cmake)

# Project globals:
project( StrusAnalyzer )
set( STRUS_MAJOR_VERSION 0 )
set( STRUS_MINOR_VERSION 14 )
set( STRUS_PATCH_VERSION 0 )
set( STRUS_VERSION ${STRUS_MAJOR_VERSION}.${STRUS_MINOR_VERSION}.${STRUS_PATCH_VERSION} )

include( cmake/dist.cmake )
include( cmake/LinuxDistribution.cmake )
include( cmake/build_rules.cmake )
include( cmake/boost.cmake)
include( cmake/intl.cmake )
include( cmake/find_strus_package.cmake )

find_strus_package( base )
set( CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" )
configure_file( "${PROJECT_SOURCE_DIR}/include/strus/versionAnalyzer.hpp.in"  "${PROJECT_SOURCE_DIR}/include/strus/versionAnalyzer.hpp"  @ONLY )
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

# Subdirectories:
add_subdirectory( 3rdParty )
add_subdirectory( src )
add_subdirectory( tests )
  
include( cmake/report_build_settings.cmake )

# Install header files:
install(DIRECTORY include/strus
          DESTINATION include
          FILES_MATCHING PATTERN "*.hpp")

include( cmake/standard_targets.cmake )
