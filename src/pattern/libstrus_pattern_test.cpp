/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Exported functions of the strus pattern matching library used as test ground (not meant for productive use)
/// \file libstrus_pattern_test.cpp
#include "strus/lib/pattern_test.hpp"
#include "strus/errorBufferInterface.hpp"
#include "patternMatcher.hpp"
#include "patternLexer.hpp"
#include "strus/base/dll_tags.hpp"
#include "private/internationalization.hpp"
#include "private/errorUtils.hpp"

using namespace strus;
static bool g_intl_initialized = false;

DLL_PUBLIC PatternMatcherInterface* strus::createPatternMatcher_test( ErrorBufferInterface* errorhnd)
{
	try
	{
		if (!g_intl_initialized)
		{
			strus::initMessageTextDomain();
			g_intl_initialized = true;
		}
		return new TestPatternMatcher( errorhnd);
	}
	CATCH_ERROR_MAP_RETURN( _TXT("error creating test token pattern matcher interface: %s"), *errorhnd, 0);
}

DLL_PUBLIC PatternLexerInterface* strus::createPatternLexer_test( ErrorBufferInterface* errorhnd)
{
	try
	{
		if (!g_intl_initialized)
		{
			strus::initMessageTextDomain();
			g_intl_initialized = true;
		}
		return new TestPatternLexer( errorhnd);
	}
	CATCH_ERROR_MAP_RETURN( _TXT("error creating test char regex matcher interface: %s"), *errorhnd, 0);
}

