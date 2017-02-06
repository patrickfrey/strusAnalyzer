/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "strus/lib/tokenizer_regex.hpp"
#include "tokenizerRegex.hpp"
#include "strus/tokenizerFunctionInterface.hpp"
#include "strus/tokenizerFunctionInstanceInterface.hpp"
#include "strus/errorBufferInterface.hpp"
#include "strus/analyzer/token.hpp"
#include "strus/base/dll_tags.hpp"
#include "private/errorUtils.hpp"
#include "private/internationalization.hpp"
#include "textwolf/charset_utf8.hpp"
#include <vector>
#include <string>

using namespace strus;
using namespace strus::analyzer;

static bool g_intl_initialized = false;

DLL_PUBLIC TokenizerFunctionInterface* strus::createTokenizer_regex( ErrorBufferInterface* errorhnd)
{
	try
	{
		if (!g_intl_initialized)
		{
			strus::initMessageTextDomain();
			g_intl_initialized = true;
		}
		return new RegexTokenizerFunction( errorhnd);
	}
	CATCH_ERROR_MAP_RETURN( _TXT("cannot create \"regex\" tokenizer: %s"), *errorhnd, 0);
}

