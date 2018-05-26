/*
 * Copyright (c) 2018 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Exported functions of the strus standard content statistics library
/// \file libstrus_contentstats_std.cpp
#include "strus/lib/contentstats_std.hpp"
#include "strus/documentClassDetectorInterface.hpp"
#include "strus/errorBufferInterface.hpp"
#include "strus/textProcessorInterface.hpp"
#include "strus/contentStatisticsInterface.hpp"
#include "contentStatistics.hpp"
#include "private/internationalization.hpp"
#include "strus/base/dll_tags.hpp"
#include "private/errorUtils.hpp"

static bool g_intl_initialized = false;

using namespace strus;

DLL_PUBLIC ContentStatisticsInterface* strus::createContentStatistics_std(
		const TextProcessorInterface* textproc,
		const DocumentClassDetectorInterface* detector,
		ErrorBufferInterface* errorhnd)
{
	try
	{
		if (!g_intl_initialized)
		{
			strus::initMessageTextDomain();
			g_intl_initialized = true;
		}
		return new ContentStatistics( textproc, detector, errorhnd);
	}
	CATCH_ERROR_MAP_RETURN( _TXT("cannot create content statistics structure: %s"), *errorhnd, 0);
}



