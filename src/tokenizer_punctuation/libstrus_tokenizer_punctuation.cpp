/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "strus/lib/tokenizer_punctuation.hpp"
#include "strus/errorBufferInterface.hpp"
#include "punctuation.hpp"
#include "strus/base/dll_tags.hpp"
#include "private/internationalization.hpp"
#include "private/errorUtils.hpp"

static bool g_intl_initialized = false;

using namespace strus;

DLL_PUBLIC TokenizerFunctionInterface* strus::createTokenizer_punctuation( ErrorBufferInterface* errorhnd)
{
	try
	{
		if (!g_intl_initialized)
		{
			strus::initMessageTextDomain();
			g_intl_initialized = true;
		}
		return punctuationTokenizer( errorhnd);
	}
	CATCH_ERROR_MAP_RETURN( _TXT("cannot create punctuation tokenizer: %s"), *errorhnd, 0);
}


