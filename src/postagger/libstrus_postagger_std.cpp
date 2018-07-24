/*
 * Copyright (c) 2018 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Library for tagging documents with part of speech tagging info fed from a 3rdParty source (e.g. standford POS tagger, google syntaxnet, etc.)
/// \file libstrus_analyzer_postagger.hpp
#include "strus/lib/postagger_std.hpp"
#include "posTagger.hpp"
#include "posTaggerData.hpp"
#include "strus/posTaggerInterface.hpp"
#include "strus/posTaggerInstanceInterface.hpp"
#include "strus/posTaggerContextInterface.hpp"
#include "strus/posTaggerDataInterface.hpp"
#include "strus/errorBufferInterface.hpp"
#include "strus/textProcessorInterface.hpp"
#include "strus/tokenizerFunctionInterface.hpp"
#include "strus/tokenizerFunctionInstanceInterface.hpp"
#include "private/errorUtils.hpp"
#include "private/internationalization.hpp"
#include "strus/base/dll_tags.hpp"
#include "strus/base/local_ptr.hpp"

static bool g_intl_initialized = false;

/// \brief strus toplevel namespace
using namespace strus;

#define COMPONENT_NAME "POS tagger"

DLL_PUBLIC PosTaggerDataInterface* strus::createPosTaggerData_standard( const TextProcessorInterface* textproc, const std::string& tokenizername, const std::vector<std::string>& tokenizerarg, ErrorBufferInterface* errorhnd)
{
	try
	{
		if (!g_intl_initialized)
		{
			strus::initMessageTextDomain();
			g_intl_initialized = true;
		}
		const TokenizerFunctionInterface* tokenizerfunc = textproc->getTokenizer( tokenizername);
		if (!tokenizerfunc) throw std::runtime_error( errorhnd->fetchError());
		strus::local_ptr<TokenizerFunctionInstanceInterface> tokenizer( tokenizerfunc->createInstance( tokenizerarg, textproc));
		PosTaggerDataInterface* rt = new PosTaggerData( tokenizer.get(), errorhnd);
		tokenizer.release();
		return rt;
	}
	CATCH_ERROR_ARG1_MAP_RETURN( _TXT("error creating data for \"%s\": %s"), COMPONENT_NAME, *errorhnd, NULL);
}

DLL_PUBLIC PosTaggerInterface* strus::createPosTagger_standard( ErrorBufferInterface* errorhnd)
{
	try
	{
		if (!g_intl_initialized)
		{
			strus::initMessageTextDomain();
			g_intl_initialized = true;
		}
		return new PosTagger( errorhnd);
	}
	CATCH_ERROR_ARG1_MAP_RETURN( _TXT("error creating \"%s\": %s"), COMPONENT_NAME, *errorhnd, NULL);
}


