/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Exported functions of the strus standard content detector library
#include "strus/lib/detector_std.hpp"
#include "strus/analyzerErrorBufferInterface.hpp"
#include "standardDocumentClassDetector.hpp"
#include "private/internationalization.hpp"
#include "private/dll_tags.hpp"
#include "private/errorUtils.hpp"

static bool g_intl_initialized = false;

using namespace strus;

DLL_PUBLIC DocumentClassDetectorInterface* strus::createDetector_std( AnalyzerErrorBufferInterface* errorhnd)
{
	try
	{
		if (!g_intl_initialized)
		{
			strus::initMessageTextDomain();
			g_intl_initialized = true;
		}
		return new StandardDocumentClassDetector( errorhnd);
	}
	CATCH_ERROR_MAP_RETURN( _TXT("cannot create query standard document detector: %s"), *errorhnd, 0);
}

