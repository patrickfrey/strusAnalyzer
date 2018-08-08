/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "strus/lib/analyzer.hpp"
#include "strus/documentAnalyzerInstanceInterface.hpp"
#include "strus/documentAnalyzerMapInterface.hpp"
#include "strus/queryAnalyzerInstanceInterface.hpp"
#include "strus/analyzer/segmenterOptions.hpp"
#include "strus/segmenterInterface.hpp"
#include "strus/errorBufferInterface.hpp"
#include "private/errorUtils.hpp"
#include "private/internationalization.hpp"
#include "documentAnalyzerInstance.hpp"
#include "documentAnalyzerMap.hpp"
#include "queryAnalyzerInstance.hpp"
#include "strus/base/dll_tags.hpp"

static bool g_intl_initialized = false;

using namespace strus;


DLL_PUBLIC DocumentAnalyzerInstanceInterface*
	strus::createDocumentAnalyzer( const TextProcessorInterface* textproc, const SegmenterInterface* segmenter, const analyzer::SegmenterOptions& opts, ErrorBufferInterface* errorhnd)
{
	try
	{
		if (!g_intl_initialized)
		{
			strus::initMessageTextDomain();
			g_intl_initialized = true;
		}
		return new DocumentAnalyzerInstance( textproc, segmenter, opts, errorhnd);
	}
	CATCH_ERROR_MAP_RETURN( _TXT("cannot create document analyzer: %s"), *errorhnd, 0);
}


DLL_PUBLIC QueryAnalyzerInstanceInterface* strus::createQueryAnalyzer( ErrorBufferInterface* errorhnd)
{
	try
	{
		if (!g_intl_initialized)
		{
			strus::initMessageTextDomain();
			g_intl_initialized = true;
		}
		return new QueryAnalyzerInstance( errorhnd);
	}
	CATCH_ERROR_MAP_RETURN( _TXT("cannot create query analyzer: %s"), *errorhnd, 0);
}

DLL_PUBLIC DocumentAnalyzerMapInterface* strus::createDocumentAnalyzerMap( const AnalyzerObjectBuilderInterface* objbuilder, ErrorBufferInterface* errorhnd)
{
	try
	{
		if (!g_intl_initialized)
		{
			strus::initMessageTextDomain();
			g_intl_initialized = true;
		}
		return new DocumentAnalyzerMap( objbuilder, errorhnd);
	}
	CATCH_ERROR_MAP_RETURN( _TXT("cannot create document analyzer map: %s"), *errorhnd, 0);
}




