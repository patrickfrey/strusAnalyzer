/*
 * Copyright (c) 2020 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "strus/lib/normalizer_decode.hpp"
#include "strus/errorBufferInterface.hpp"
#include "normalizerDecode.hpp"
#include "strus/base/dll_tags.hpp"
#include "private/internationalization.hpp"
#include "private/errorUtils.hpp"

static bool g_intl_initialized = false;

using namespace strus;


DLL_PUBLIC NormalizerFunctionInterface* strus::createNormalizer_decode_xmlent( ErrorBufferInterface* errorhnd)
{
	try
	{
		if (!g_intl_initialized)
		{
			strus::initMessageTextDomain();
			g_intl_initialized = true;
		}
		return new DecodeXmlEntityNormalizerFunction( errorhnd);
	}
	CATCH_ERROR_ARG1_MAP_RETURN( _TXT("cannot create '%s' normalizer: %s"), "decode_xmlent", *errorhnd, 0);
}

DLL_PUBLIC NormalizerFunctionInterface* strus::createNormalizer_decode_url( ErrorBufferInterface* errorhnd)
{
	try
	{
		if (!g_intl_initialized)
		{
			strus::initMessageTextDomain();
			g_intl_initialized = true;
		}
		return new DecodeUrlEntityNormalizerFunction( errorhnd);
	}
	CATCH_ERROR_ARG1_MAP_RETURN( _TXT("cannot create '%s' normalizer: %s"), "decode_url", *errorhnd, 0);
}





