/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "strus/lib/textproc.hpp"
#include "strus/lib/normalizer_snowball.hpp"
#include "strus/lib/normalizer_dictmap.hpp"
#include "strus/lib/normalizer_charconv.hpp"
#include "strus/lib/normalizer_dateconv.hpp"
#include "strus/lib/tokenizer_punctuation.hpp"
#include "strus/lib/tokenizer_word.hpp"
#include "strus/lib/aggregator_vsm.hpp"
#include "strus/errorBufferInterface.hpp"
#include "strus/normalizerFunctionInterface.hpp"
#include "strus/tokenizerFunctionInterface.hpp"
#include "textProcessor.hpp"
#include "private/dll_tags.hpp"
#include "private/internationalization.hpp"
#include <stdexcept>

using namespace strus;

DLL_PUBLIC strus::TextProcessorInterface*
	strus::createTextProcessor( ErrorBufferInterface* errorhnd)
{
	TextProcessor* rt = 0;
	NormalizerFunctionInterface* nrm;
	TokenizerFunctionInterface* tkn;
	AggregatorFunctionInterface* agr;

	rt = new TextProcessor( errorhnd);
	if (0==(nrm = createNormalizer_snowball( errorhnd)))
	{
		errorhnd->explain( _TXT("error creating text processor: %s"));
		return 0;
	}
	rt->defineNormalizer( "stem", nrm);
	if (0==(nrm = createNormalizer_dictmap( errorhnd)))
	{
		errorhnd->explain( _TXT("error creating text processor: %s"));
		return 0;
	}
	rt->defineNormalizer( "dictmap", nrm);
	if (0==(nrm = createNormalizer_lowercase( errorhnd)))
	{
		errorhnd->explain( _TXT("error creating text processor: %s"));
		return 0;
	}
	rt->defineNormalizer( "lc", nrm);
	if (0==(nrm = createNormalizer_uppercase( errorhnd)))
	{
		errorhnd->explain( _TXT("error creating text processor: %s"));
		return 0;
	}
	rt->defineNormalizer( "uc", nrm);
	if (0==(nrm = createNormalizer_convdia( errorhnd)))
	{
		errorhnd->explain( _TXT("error creating text processor: %s"));
		return 0;
	}
	rt->defineNormalizer( "convdia", nrm);
	if (0==(nrm = createNormalizer_date2int( errorhnd)))
	{
		errorhnd->explain( _TXT("error creating text processor: %s"));
		return 0;
	}
	rt->defineNormalizer( "date2int", nrm);
	if (0==(tkn = createTokenizer_punctuation( errorhnd)))
	{
		errorhnd->explain( _TXT("error creating text processor: %s"));
		return 0;
	}
	rt->defineTokenizer( "punctuation", tkn);
	if (0==(tkn = createTokenizer_word( errorhnd)))
	{
		errorhnd->explain( _TXT("error creating text processor: %s"));
		return 0;
	}
	rt->defineTokenizer( "word", tkn);
	if (0==(tkn = createTokenizer_whitespace( errorhnd)))
	{
		errorhnd->explain( _TXT("error creating text processor: %s"));
		return 0;
	}
	rt->defineTokenizer( "split", tkn);
	if (0==(agr = createAggregator_sumSquareTf( errorhnd)))
	{
		errorhnd->explain( _TXT("error creating text processor: %s"));
		return 0;
	}
	rt->defineAggregator( "sumsquaretf", agr);
	return rt;
}


