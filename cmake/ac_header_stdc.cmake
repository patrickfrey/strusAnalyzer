# analogue of AC_HEADER_STDC automake macro.
# copied from https://github.com/sphinxsearch/sphinx/cmake/ac_header_stdc.cmake
# and https://github.com/sphinxsearch/sphinx/cmake/helpers.cmake

# - Check if the DIR symbol exists like in AC_HEADER_DIRENT.
# CHECK_DIRSYMBOL_EXISTS(FILES VARIABLE)
#
# FILES - include files to check
# VARIABLE - variable to return result
#
# This module is a small but important variation on CheckSymbolExists.cmake.
# The symbol always searched for is DIR, and the test programme follows
# the AC_HEADER_DIRENT test programme rather than the CheckSymbolExists.cmake
# test programme which always fails since DIR tends to be typedef'd
# rather than #define'd.
#
# The following variables may be set before calling this macro to
# modify the way the check is run:
#
# CMAKE_REQUIRED_FLAGS = string of compile command line flags
# CMAKE_REQUIRED_DEFINITIONS = list of macros to define (-DFOO=bar)
# CMAKE_REQUIRED_INCLUDES = list of include directories
# CMAKE_REQUIRED_LIBRARIES = list of libraries to link

MACRO(CHECK_DIRSYMBOL_EXISTS FILES VARIABLE)
	IF(NOT DEFINED ${VARIABLE})
		SET(CMAKE_CONFIGURABLE_FILE_CONTENT "/* */\n")
		SET(MACRO_CHECK_DIRSYMBOL_EXISTS_FLAGS ${CMAKE_REQUIRED_FLAGS})
		IF(CMAKE_REQUIRED_LIBRARIES)
			SET(CHECK_DIRSYMBOL_EXISTS_LIBS
				"-DLINK_LIBRARIES:STRING=${CMAKE_REQUIRED_LIBRARIES}")
		ELSE(CMAKE_REQUIRED_LIBRARIES)
			SET(CHECK_DIRSYMBOL_EXISTS_LIBS)
		ENDIF(CMAKE_REQUIRED_LIBRARIES)
		IF(CMAKE_REQUIRED_INCLUDES)
			SET(CMAKE_DIRSYMBOL_EXISTS_INCLUDES
				"-DINCLUDE_DIRECTORIES:STRING=${CMAKE_REQUIRED_INCLUDES}")
		ELSE(CMAKE_REQUIRED_INCLUDES)
			SET(CMAKE_DIRSYMBOL_EXISTS_INCLUDES)
		ENDIF(CMAKE_REQUIRED_INCLUDES)
		FOREACH(FILE ${FILES})
			SET(CMAKE_CONFIGURABLE_FILE_CONTENT
				"${CMAKE_CONFIGURABLE_FILE_CONTENT}#include <${FILE}>\n")
		ENDFOREACH(FILE)
		SET(CMAKE_CONFIGURABLE_FILE_CONTENT
			"${CMAKE_CONFIGURABLE_FILE_CONTENT}\nint main()\n{if ((DIR *) 0) return 0;}\n")

		CONFIGURE_FILE("${CMAKE_ROOT}/Modules/CMakeConfigurableFile.in"
			"${CMAKE_BINARY_DIR}/CMakeFiles/CMakeTmp/CheckDIRSymbolExists.c" @ONLY)

		MESSAGE(STATUS "Looking for DIR in ${FILES}")
		TRY_COMPILE(${VARIABLE}
			${CMAKE_BINARY_DIR}
			${CMAKE_BINARY_DIR}/CMakeFiles/CMakeTmp/CheckDIRSymbolExists.c
			COMPILE_DEFINITIONS ${CMAKE_REQUIRED_DEFINITIONS}
			CMAKE_FLAGS
			-DCOMPILE_DEFINITIONS:STRING=${MACRO_CHECK_DIRSYMBOL_EXISTS_FLAGS}
			"${CHECK_DIRSYMBOL_EXISTS_LIBS}"
			"${CMAKE_DIRSYMBOL_EXISTS_INCLUDES}"
			OUTPUT_VARIABLE OUTPUT)
		IF(${VARIABLE})
			MESSAGE(STATUS "Looking for DIR in ${FILES} - found")
			SET(${VARIABLE} 1 CACHE INTERNAL "Have symbol DIR")
			FILE(APPEND ${CMAKE_BINARY_DIR}/CMakeFiles/CMakeOutput.log
				"Determining if the DIR symbol is defined as in AC_HEADER_DIRENT "
				"passed with the following output:\n"
				"${OUTPUT}\nFile ${CMAKE_BINARY_DIR}/CMakeFiles/CMakeTmp/CheckDIRSymbolExists.c:\n"
				"${CMAKE_CONFIGURABLE_FILE_CONTENT}\n")
		ELSE(${VARIABLE})
			MESSAGE(STATUS "Looking for DIR in ${FILES} - not found.")
			SET(${VARIABLE} "" CACHE INTERNAL "Have symbol DIR")
			FILE(APPEND ${CMAKE_BINARY_DIR}/CMakeFiles/CMakeError.log
				"Determining if the DIR symbol is defined as in AC_HEADER_DIRENT "
				"failed with the following output:\n"
				"${OUTPUT}\nFile ${CMAKE_BINARY_DIR}/CMakeFiles/CMakeTmp/CheckDIRSymbolExists.c:\n"
				"${CMAKE_CONFIGURABLE_FILE_CONTENT}\n")
		ENDIF(${VARIABLE})
	ENDIF(NOT DEFINED ${VARIABLE})
ENDMACRO(CHECK_DIRSYMBOL_EXISTS)

include(CheckCXXSourceCompiles)
macro(check_prototype_exists _SYMBOL _HEADER _RESULT)
	set(_INCLUDE_FILES)
	foreach(it ${_HEADER})
		set(_INCLUDE_FILES "${_INCLUDE_FILES}#include <${it}>\n")
	endforeach(it)

	set(_CHECK_PROTO_EXISTS_SOURCE_CODE "
${_INCLUDE_FILES}
void cmakeRequireSymbol(int dummy,...){(void)dummy;}
int main()
{
#ifndef ${_SYMBOL}
#ifndef _MSC_VER
cmakeRequireSymbol(0,&${_SYMBOL});
#else
char i = sizeof(&${_SYMBOL});
#endif
#endif
return 0;
}
")
	CHECK_CXX_SOURCE_COMPILES("${_CHECK_PROTO_EXISTS_SOURCE_CODE}" ${_RESULT})
endmacro(check_prototype_exists _SYMBOL _HEADER _RESULT)

include(CheckIncludeFiles)
check_include_files("dlfcn.h;stdint.h;stddef.h;inttypes.h;stdlib.h;strings.h;string.h;float.h" StandardHeadersExist)
if(StandardHeadersExist)
	check_prototype_exists(free stdlib.h freeExists)
	if(freeExists)
		message(STATUS "ANSI C header files - found")
		set(STDC_HEADERS 1 CACHE INTERNAL "System has ANSI C header files")
		set(HAVE_STDINT_H 1)
		set(HAVE_INTTYPES_H 1)
	endif(freeExists)
endif(StandardHeadersExist)

if(NOT STDC_HEADERS)
	message(STATUS "ANSI C header files - not found")
	set(STDC_HEADERS 0 CACHE INTERNAL "System has ANSI C header files")
endif(NOT STDC_HEADERS)

check_dirsymbol_exists("sys/types.h" HAVE_SYS_TYPES_H)
check_include_files("stdint.h;inttypes.h" StandardHeadersExist)
