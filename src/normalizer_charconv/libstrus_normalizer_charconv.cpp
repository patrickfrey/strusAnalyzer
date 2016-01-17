/*
---------------------------------------------------------------------
    The C++ library strus implements basic operations to build
    a search engine for structured search on unstructured data.

    Copyright (C) 2015 Patrick Frey

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License as published by the Free Software Foundation; either
    version 3 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

--------------------------------------------------------------------

	The latest version of strus can be found at 'http://github.com/patrickfrey/strus'
	For documentation see 'http://patrickfrey.github.com/strus'

--------------------------------------------------------------------
*/
#include "strus/lib/normalizer_charconv.hpp"
#include "strus/analyzerErrorBufferInterface.hpp"
#include "normalizerCharConv.hpp"
#include "private/dll_tags.hpp"
#include "private/internationalization.hpp"
#include "private/errorUtils.hpp"

static bool g_intl_initialized = false;

using namespace strus;


DLL_PUBLIC NormalizerFunctionInterface* strus::createNormalizer_lowercase( AnalyzerErrorBufferInterface* errorhnd)
{
	try
	{
		if (!g_intl_initialized)
		{
			strus::initMessageTextDomain();
			g_intl_initialized = true;
		}
		return new LowercaseNormalizerFunction( errorhnd);
	}
	CATCH_ERROR_MAP_RETURN( _TXT("cannot create lowercase character normalizer: %s"), *errorhnd, 0);
}

DLL_PUBLIC NormalizerFunctionInterface* strus::createNormalizer_uppercase( AnalyzerErrorBufferInterface* errorhnd)
{
	try
	{
		if (!g_intl_initialized)
		{
			strus::initMessageTextDomain();
			g_intl_initialized = true;
		}
		return new UppercaseNormalizerFunction( errorhnd);
	}
	CATCH_ERROR_MAP_RETURN( _TXT("cannot create uppercase character normalizer: %s"), *errorhnd, 0);
}

DLL_PUBLIC NormalizerFunctionInterface* strus::createNormalizer_convdia( AnalyzerErrorBufferInterface* errorhnd)
{
	try
	{
		if (!g_intl_initialized)
		{
			strus::initMessageTextDomain();
			g_intl_initialized = true;
		}
		return new DiacriticalNormalizerFunction( errorhnd);
	}
	CATCH_ERROR_MAP_RETURN( _TXT("cannot create diacritical character normalizer: %s"), *errorhnd, 0);
}



