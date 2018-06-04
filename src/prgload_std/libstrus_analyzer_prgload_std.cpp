/*
 * Copyright (c) 2018 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Exported functions for the program loader of the analyzer (load program in a domain specific language)
#include "strus/lib/analyzer_prgload_std.hpp"
#include "strus/errorBufferInterface.hpp"
#include "strus/textProcessorInterface.hpp"
#include "private/internationalization.hpp"
#include "strus/base/dll_tags.hpp"
#include "strus/base/fileio.hpp"
#include "private/errorUtils.hpp"
#include "programLoader.hpp"
#include "patternMatchProgramParser.hpp"
#include <cstring>

static bool g_intl_initialized = false;

using namespace strus;

DLL_PUBLIC bool strus::is_DocumentAnalyzer_programfile( const TextProcessorInterface* textproc, const std::string& filename, ErrorBufferInterface* errorhnd)
{
	try
	{
		if (!g_intl_initialized)
		{
			strus::initMessageTextDomain();
			g_intl_initialized = true;
		}
		return isDocumentAnalyzerProgramFile( textproc, filename, errorhnd);
	}
	CATCH_ERROR_MAP_RETURN( _TXT("cannot determine if file is an analyzer DSL program: %s"), *errorhnd, 0);
}

bool strus::is_DocumentAnalyzer_program( const std::string& source, ErrorBufferInterface* errorhnd)
{
	try
	{
		if (!g_intl_initialized)
		{
			strus::initMessageTextDomain();
			g_intl_initialized = true;
		}
		return isDocumentAnalyzerProgramSource( source, errorhnd);
	}
	CATCH_ERROR_MAP_RETURN( _TXT("cannot determine if file is an analyzer DSL program: %s"), *errorhnd, false);
}

DLL_PUBLIC bool strus::load_DocumentAnalyzer_program_std( DocumentAnalyzerInstanceInterface* analyzer, const TextProcessorInterface* textproc, const std::string& content, ErrorBufferInterface* errorhnd)
{
	try
	{
		if (!g_intl_initialized)
		{
			strus::initMessageTextDomain();
			g_intl_initialized = true;
		}
		bool allowIncludes = false;
		return loadDocumentAnalyzerProgramSource( analyzer, textproc, content, allowIncludes, errorhnd);
	}
	CATCH_ERROR_MAP_RETURN( _TXT("cannot load analyzer from DSL program: %s"), *errorhnd, false);
}

DLL_PUBLIC bool strus::load_DocumentAnalyzer_programfile_std( DocumentAnalyzerInstanceInterface* analyzer, const TextProcessorInterface* textproc, const std::string& filename, ErrorBufferInterface* errorhnd)
{
	try
	{
		if (!g_intl_initialized)
		{
			strus::initMessageTextDomain();
			g_intl_initialized = true;
		}
		return loadDocumentAnalyzerProgramFile( analyzer, textproc, filename, errorhnd);
	}
	CATCH_ERROR_ARG1_MAP_RETURN( _TXT("cannot load analyzer program file %s: %s"), filename.c_str(), *errorhnd, false);
}

DLL_PUBLIC bool strus::load_DocumentAnalyzerMap_program(
		DocumentAnalyzerMapInterface* analyzermap,
		const TextProcessorInterface* textproc,
		const std::string& source,
		ErrorBufferInterface* errorhnd)
{
	try
	{
		if (!g_intl_initialized)
		{
			strus::initMessageTextDomain();
			g_intl_initialized = true;
		}
		return loadDocumentAnalyzerMapSource( analyzermap, textproc, source, errorhnd);
	}
	CATCH_ERROR_MAP_RETURN( _TXT("failed to load analyzer map source: %s"), *errorhnd, 0);
}

DLL_PUBLIC bool strus::load_DocumentAnalyzerMap_programfile(
		DocumentAnalyzerMapInterface* analyzermap,
		const TextProcessorInterface* textproc,
		const std::string& filename,
		ErrorBufferInterface* errorhnd)
{
	try
	{
		if (!g_intl_initialized)
		{
			strus::initMessageTextDomain();
			g_intl_initialized = true;
		}
		return loadDocumentAnalyzerMapFile( analyzermap, textproc, filename, errorhnd);
	}
	CATCH_ERROR_MAP_RETURN( _TXT("failed to load analyzer map file: %s"), *errorhnd, 0);
}

DLL_PUBLIC bool strus::load_PatternMatcher_program(
		const TextProcessorInterface* textproc,
		PatternTermFeederInstanceInterface* feeder,
		PatternMatcherInstanceInterface* matcher,
		const std::string& content,
		ErrorBufferInterface* errorhnd)
{
	try
	{
		PatternMatcherProgramParser parser( feeder, matcher, errorhnd);
		if (!parser.load( content)) return false;
		return true;
	}
	CATCH_ERROR_MAP_RETURN( _TXT("failed to load pattern matcher program source: %s"), *errorhnd, 0);
}

DLL_PUBLIC bool strus::load_PatternMatcher_programfile(
		const TextProcessorInterface* textproc,
		PatternTermFeederInstanceInterface* feeder,
		PatternMatcherInstanceInterface* matcher,
		const std::string& filename,
		ErrorBufferInterface* errorhnd)
{
	try
	{
		std::string source;
		std::string filepath = textproc->getResourcePath( filename);
		if (filepath.empty() && errorhnd->hasError()) throw std::runtime_error( errorhnd->fetchError());
		int ec = strus::readFile( filepath, source);
		if (ec) throw strus::runtime_error(_TXT( "failed to read pattern match file '%s': %s"), filepath.c_str(), ::strerror(ec));
		PatternMatcherProgramParser parser( feeder, matcher, errorhnd);
		if (!parser.load( source)) return false;
		return true;
	}
	CATCH_ERROR_ARG1_MAP_RETURN( _TXT("failed to load pattern matcher program file '%s': %s"), filename.c_str(), *errorhnd, 0);
}

DLL_PUBLIC bool strus::load_PatternMatcher_program(
		const TextProcessorInterface* textproc,
		PatternLexerInstanceInterface* lexer,
		PatternMatcherInstanceInterface* matcher,
		const std::string& content,
		ErrorBufferInterface* errorhnd)
{
	try
	{
		PatternMatcherProgramParser parser( lexer, matcher, errorhnd);
		if (!parser.load( content)) return false;
		return true;
	}
	CATCH_ERROR_MAP_RETURN( _TXT("failed to load pattern matcher program source: %s"), *errorhnd, 0);
}

DLL_PUBLIC bool strus::load_PatternMatcher_programfile(
		const TextProcessorInterface* textproc,
		PatternLexerInstanceInterface* lexer,
		PatternMatcherInstanceInterface* matcher,
		const std::string& filename,
		ErrorBufferInterface* errorhnd)
{
	try
	{
		std::string source;
		std::string filepath = textproc->getResourcePath( filename);
		if (filepath.empty() && errorhnd->hasError()) throw std::runtime_error( errorhnd->fetchError());
		int ec = strus::readFile( filepath, source);
		if (ec) throw strus::runtime_error(_TXT( "failed to read pattern match file '%s': %s"), filepath.c_str(), ::strerror(ec));
		PatternMatcherProgramParser parser( lexer, matcher, errorhnd);
		if (!parser.load( source)) return false;
		return true;
	}
	CATCH_ERROR_ARG1_MAP_RETURN( _TXT("failed to load pattern matcher program file '%s': %s"), filename.c_str(), *errorhnd, 0);
}


