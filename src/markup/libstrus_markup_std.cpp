/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Exported functions of the strus standard markup library
/// \file libstrus_markup_std.cpp
#include "strus/lib/markup_std.hpp"
#include "strus/errorBufferInterface.hpp"
#include "tokenMarkup.hpp"
#include "strus/base/dll_tags.hpp"
#include "private/internationalization.hpp"
#include "private/errorUtils.hpp"

using namespace strus;
static bool g_intl_initialized = false;

DLL_PUBLIC TokenMarkupInstanceInterface* strus::createTokenMarkupInstance_standard(
		ErrorBufferInterface* errorhnd)
{
	try
	{
		if (!g_intl_initialized)
		{
			strus::initMessageTextDomain();
			g_intl_initialized = true;
		}
		return new TokenMarkupInstance( errorhnd);
	}
	CATCH_ERROR_MAP_RETURN( _TXT("error creating token markup interface: %s"), *errorhnd, 0);
}


