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
#include "strus/lib/normalizer_ngram.hpp"
#include "strus/lib/normalizer_regex.hpp"
#include "strus/lib/normalizer_wordjoin.hpp"
#include "strus/lib/normalizer_trim.hpp"
#include "strus/lib/normalizer_substrindex.hpp"
#include "strus/lib/normalizer_entityid.hpp"
#include "strus/lib/tokenizer_punctuation.hpp"
#include "strus/lib/tokenizer_word.hpp"
#include "strus/lib/tokenizer_regex.hpp"
#include "strus/lib/tokenizer_textcat.hpp"
#include "strus/lib/aggregator_vsm.hpp"
#include "strus/lib/aggregator_set.hpp"
#include "strus/errorBufferInterface.hpp"
#include "strus/normalizerFunctionInterface.hpp"
#include "strus/tokenizerFunctionInterface.hpp"
#include "textProcessor.hpp"
#include "strus/base/dll_tags.hpp"
#include "private/internationalization.hpp"
#include <stdexcept>

using namespace strus;

DLL_PUBLIC strus::TextProcessorInterface*
	strus::createTextProcessor( const FileLocatorInterface* filelocator, ErrorBufferInterface* errorhnd)
{
	TextProcessor* rt = 0;
	NormalizerFunctionInterface* nrm;
	TokenizerFunctionInterface* tkn;
	AggregatorFunctionInterface* agr;

	rt = new TextProcessor( filelocator, errorhnd);
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
	if (0==(nrm = createNormalizer_charselect( errorhnd)))
	{
		errorhnd->explain( _TXT("error creating text processor: %s"));
		return 0;
	}
	rt->defineNormalizer( "charselect", nrm);
	if (0==(nrm = createNormalizer_date2int( errorhnd)))
	{
		errorhnd->explain( _TXT("error creating text processor: %s"));
		return 0;
	}
	rt->defineNormalizer( "date2int", nrm);
	if (0==(nrm = createNormalizer_ngram( errorhnd)))
	{
		errorhnd->explain( _TXT("error creating text processor: %s"));
		return 0;
	}
	rt->defineNormalizer( "ngram", nrm);
	if (0==(nrm = createNormalizer_regex( errorhnd)))
	{
		errorhnd->explain( _TXT("error creating text processor: %s"));
		return 0;
	}
	rt->defineNormalizer( "regex", nrm);
	if (0==(nrm = createNormalizer_wordjoin( errorhnd)))
	{
		errorhnd->explain( _TXT("error creating text processor: %s"));
		return 0;
	}
	rt->defineNormalizer( "wordjoin", nrm);
	if (0==(nrm = createNormalizer_trim( errorhnd)))
	{
		errorhnd->explain( _TXT("error creating text processor: %s"));
		return 0;
	}
	rt->defineNormalizer( "trim", nrm);
	if (0==(nrm = createNormalizer_substrindex( errorhnd)))
	{
		errorhnd->explain( _TXT("error creating text processor: %s"));
		return 0;
	}
	rt->defineNormalizer( "substrindex", nrm);
	if (0==(nrm = createNormalizer_substrmap( errorhnd)))
	{
		errorhnd->explain( _TXT("error creating text processor: %s"));
		return 0;
	}
	rt->defineNormalizer( "substrmap", nrm);
	if (0==(nrm = createNormalizer_entityid( errorhnd)))
	{
		errorhnd->explain( _TXT("error creating text processor: %s"));
		return 0;
	}
	rt->defineNormalizer( "entityid", nrm);

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
	if (0==(tkn = createTokenizer_regex( errorhnd)))
	{
		errorhnd->explain( _TXT("error creating text processor: %s"));
		return 0;
	}
	rt->defineTokenizer( "regex", tkn);
	if (0==(tkn = createTokenizer_whitespace( errorhnd)))
	{
		errorhnd->explain( _TXT("error creating text processor: %s"));
		return 0;
	}
	rt->defineTokenizer( "split", tkn);
	if (0==(tkn = createTokenizer_langtoken( errorhnd)))
	{
		errorhnd->explain( _TXT("error creating text processor: %s"));
		return 0;
	}
	rt->defineTokenizer( "langtoken", tkn);
	if (0==(tkn = createTokenizer_textcat( rt, errorhnd)))
	{
		errorhnd->explain( _TXT("error creating text processor: %s"));
		return 0;
	}
	rt->defineTokenizer( "textcat", tkn);
	if (0==(agr = createAggregator_sumSquareTf( errorhnd)))
	{
		errorhnd->explain( _TXT("error creating text processor: %s"));
		return 0;
	}
	rt->defineAggregator( "sumsquaretf", agr);
	if (0==(agr = createAggregator_typeset( errorhnd)))
	{
		errorhnd->explain( _TXT("error creating text processor: %s"));
		return 0;
	}
	rt->defineAggregator( "typeset", agr);
	if (0==(agr = createAggregator_valueset( errorhnd)))
	{
		errorhnd->explain( _TXT("error creating text processor: %s"));
		return 0;
	}
	rt->defineAggregator( "valueset", agr);
	return rt;
}


