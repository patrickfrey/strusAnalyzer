/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "strus/lib/analyzer.hpp"
#include "strus/documentAnalyzerInterface.hpp"
#include "strus/queryAnalyzerInterface.hpp"
#include "strus/analyzer/segmenterOptions.hpp"
#include "strus/segmenterInterface.hpp"
#include "strus/errorBufferInterface.hpp"
#include "private/errorUtils.hpp"
#include "private/internationalization.hpp"
#include "documentAnalyzer.hpp"
#include "queryAnalyzer.hpp"
#include "strus/base/dll_tags.hpp"

static bool g_intl_initialized = false;

using namespace strus;


DLL_PUBLIC DocumentAnalyzerInterface*
	strus::createDocumentAnalyzer( const SegmenterInterface* segmenter, const analyzer::SegmenterOptions& opts, ErrorBufferInterface* errorhnd)
{
	try
	{
		if (!g_intl_initialized)
		{
			strus::initMessageTextDomain();
			g_intl_initialized = true;
		}
		return new DocumentAnalyzer( segmenter, opts, errorhnd);
	}
	CATCH_ERROR_MAP_RETURN( _TXT("cannot create document analyzer: %s"), *errorhnd, 0);
}


DLL_PUBLIC QueryAnalyzerInterface* strus::createQueryAnalyzer( ErrorBufferInterface* errorhnd)
{
	try
	{
		if (!g_intl_initialized)
		{
			strus::initMessageTextDomain();
			g_intl_initialized = true;
		}
		return new QueryAnalyzer( errorhnd);
	}
	CATCH_ERROR_MAP_RETURN( _TXT("cannot create query analyzer: %s"), *errorhnd, 0);
}





