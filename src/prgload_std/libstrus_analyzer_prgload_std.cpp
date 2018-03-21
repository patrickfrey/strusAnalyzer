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
#include "private/internationalization.hpp"
#include "strus/base/dll_tags.hpp"
#include "private/errorUtils.hpp"
#include "programLoader.hpp"

static bool g_intl_initialized = false;

using namespace strus;

DLL_PUBLIC bool strus::load_DocumentAnalyzer_program_std( DocumentAnalyzerInterface* analyzer, const TextProcessorInterface* textproc, const std::string& content, std::vector<std::string>& warnings, ErrorBufferInterface* errorhnd)
{
	try
	{
		if (!g_intl_initialized)
		{
			strus::initMessageTextDomain();
			g_intl_initialized = true;
		}
		bool allowIncludes = true;
		return loadDocumentAnalyzerProgram( analyzer, textproc, content, allowIncludes, warnings, errorhnd);
	}
	CATCH_ERROR_MAP_RETURN( _TXT("cannot load analyzer from DSL program: %s"), *errorhnd, 0);
}

