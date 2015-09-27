/*
---------------------------------------------------------------------
    The C++ library strus implements basic operations to build
    a search engine for structured search on unstructured data.

    Copyright (C) 2013,2014 Patrick Frey

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

--------------------------------------------------------------------

	The latest version of strus can be found at 'http://github.com/patrickfrey/strus'
	For documentation see 'http://patrickfrey.github.com/strus'

--------------------------------------------------------------------
*/
#include "strus/lib/textproc.hpp"
#include "strus/lib/normalizer_snowball.hpp"
#include "strus/lib/normalizer_dictmap.hpp"
#include "strus/lib/normalizer_charconv.hpp"
#include "strus/lib/normalizer_dateconv.hpp"
#include "strus/lib/tokenizer_punctuation.hpp"
#include "strus/lib/tokenizer_word.hpp"
#include "strus/analyzerErrorBufferInterface.hpp"
#include "strus/normalizerFunctionInterface.hpp"
#include "strus/tokenizerFunctionInterface.hpp"
#include "textProcessor.hpp"
#include "private/dll_tags.hpp"
#include "private/internationalization.hpp"
#include <stdexcept>

using namespace strus;

DLL_PUBLIC strus::TextProcessorInterface*
	strus::createTextProcessor( AnalyzerErrorBufferInterface* errorhnd)
{
	TextProcessor* rt = 0;
	try
	{
		NormalizerFunctionInterface* nrm;
		TokenizerFunctionInterface* tkn;

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
		return rt;
	}
	catch (const std::bad_alloc&)
	{
		if (rt) delete rt;
		errorhnd->report( _TXT("out of memory creating textprocessor"));
		return 0;
	}
	catch (const std::runtime_error& err)
	{
		if (rt) delete rt;
		errorhnd->report( _TXT("error creating textprocessor: %s"), err.what());
		return 0;
	}
}


