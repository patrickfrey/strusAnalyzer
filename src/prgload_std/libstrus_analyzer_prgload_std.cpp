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
#include <cstring>

static bool g_intl_initialized = false;

using namespace strus;

DLL_PUBLIC bool strus::is_DocumentAnalyzer_programfile( const TextProcessorInterface* textproc, const std::string& filename, ErrorBufferInterface* errorhnd)
{
	try
	{
		return isDocumentAnalyzerProgram( textproc, filename, errorhnd);
	}
	CATCH_ERROR_MAP_RETURN( _TXT("cannot determine if file is an analyzer DSL program: %s"), *errorhnd, 0);
}

DLL_PUBLIC bool strus::load_DocumentAnalyzer_program_std( DocumentAnalyzerInterface* analyzer, const TextProcessorInterface* textproc, const std::string& content, ErrorBufferInterface* errorhnd)
{
	try
	{
		if (!g_intl_initialized)
		{
			strus::initMessageTextDomain();
			g_intl_initialized = true;
		}
		bool allowIncludes = false;
		return loadDocumentAnalyzerProgram( analyzer, textproc, content, allowIncludes, errorhnd);
	}
	CATCH_ERROR_MAP_RETURN( _TXT("cannot load analyzer from DSL program: %s"), *errorhnd, 0);
}

DLL_PUBLIC bool strus::load_DocumentAnalyzer_programfile_std( DocumentAnalyzerInterface* analyzer, const TextProcessorInterface* textproc, const std::string& filename, ErrorBufferInterface* errorhnd)
{
	try
	{
		if (!g_intl_initialized)
		{
			strus::initMessageTextDomain();
			g_intl_initialized = true;
		}
		if (filename.empty()) throw strus::runtime_error( _TXT("program file name is empty"));
		std::string filepath = textproc->getResourcePath( filename);
		if (filepath.empty()) throw std::runtime_error(_TXT("failed to find program path"));
		std::string content;
		int ec = strus::readFile( filepath, content);
		if (ec) throw strus::runtime_error(_TXT("failed to read program file: %s"), ::strerror(ec));

		bool allowIncludes = true;
		return loadDocumentAnalyzerProgram( analyzer, textproc, content, allowIncludes, errorhnd);
	}
	CATCH_ERROR_ARG1_MAP_RETURN( _TXT("cannot load analyzer from DSL program file %s: %s"), filename.c_str(), *errorhnd, 0);
}

DLL_PUBLIC bool strus::load_DocumentAnalyzerMap(
		std::vector<AnalyzerMapElement>& mapdef,
		const std::string& source,
		ErrorBufferInterface* errorhnd)
{
	return false;
}



