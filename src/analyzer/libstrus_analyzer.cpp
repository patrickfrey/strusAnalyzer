/*
---------------------------------------------------------------------
    The C++ library strus implements basic operations to build
    a search engine for structured search on unstructured data.

    Copyright (C) 2013,2014 Patrick Frey

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

--------------------------------------------------------------------

	The latest version of strus can be found at 'http://github.com/patrickfrey/strus'
	For documentation see 'http://patrickfrey.github.com/strus'

--------------------------------------------------------------------
*/
#include "strus/lib/analyzer.hpp"
#include "strus/documentAnalyzerInterface.hpp"
#include "strus/queryAnalyzerInterface.hpp"
#include "strus/segmenterInterface.hpp"
#include "strus/analyzerErrorBufferInterface.hpp"
#include "private/errorUtils.hpp"
#include "private/internationalization.hpp"
#include "documentAnalyzer.hpp"
#include "queryAnalyzer.hpp"
#include "private/dll_tags.hpp"

static bool g_intl_initialized = false;

using namespace strus;


DLL_PUBLIC DocumentAnalyzerInterface*
	strus::createDocumentAnalyzer( SegmenterInterface* segmenter, AnalyzerErrorBufferInterface* errorhnd)
{
	try
	{
		if (!g_intl_initialized)
		{
			strus::initMessageTextDomain();
			g_intl_initialized = true;
		}
		return new DocumentAnalyzer( segmenter, errorhnd);
	}
	CATCH_ERROR_MAP_RETURN( _TXT("cannot create document analyzer: %s"), *errorhnd, 0);
}


DLL_PUBLIC QueryAnalyzerInterface* strus::createQueryAnalyzer( AnalyzerErrorBufferInterface* errorhnd)
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





