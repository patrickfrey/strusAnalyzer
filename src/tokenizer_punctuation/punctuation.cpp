/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "punctuation.hpp"
#include "strus/tokenizerFunctionInterface.hpp"
#include "strus/tokenizerFunctionInstanceInterface.hpp"
#include "strus/errorBufferInterface.hpp"
#include "strus/base/string_conv.hpp"
#include "private/errorUtils.hpp"
#include "private/internationalization.hpp"
#include "punctuation_de.hpp"
#include "punctuation_en.hpp"
#include <cstring>
#include <iostream>
#include <stdexcept>

using namespace strus;
using namespace strus::analyzer;

class PunctuationTokenizerFunction
	:public TokenizerFunctionInterface
{
public:
	explicit PunctuationTokenizerFunction( ErrorBufferInterface* errorhnd_)
		:m_errorhnd(errorhnd_){}

	virtual TokenizerFunctionInstanceInterface* createInstance( const std::vector<std::string>& args, const TextProcessorInterface*) const
	{
		try
		{
			if (args.size() > 2)
			{
				m_errorhnd->report( ErrorCodeInvalidArgument, _TXT("too many arguments for punctuation tokenizer (1st language 2nd optional punctuation characters)"));
				return 0;
			}
			if (args.size() < 1)
			{
				m_errorhnd->report( ErrorCodeIncompleteDefinition, _TXT("too few arguments for punctuation tokenizer (language as mandatory argument expected)"));
				return 0;
			}
			const char* punctChar = 0;
			if (args.size() == 2)
			{
				punctChar = args[1].c_str();
			}
			if (strus::caseInsensitiveEquals( args[0], "de"))
			{
				return new PunctuationTokenizerInstance_de( punctChar, m_errorhnd);
			}
			else if (strus::caseInsensitiveEquals( args[0], "en"))
			{
				return new PunctuationTokenizerInstance_en( punctChar, m_errorhnd);
			}
			else
			{
				m_errorhnd->report( ErrorCodeNotImplemented, _TXT("unsupported language passed to punctuation tokenizer ('%s')"), args[0].c_str());
				return 0;
			}
		}
		CATCH_ERROR_MAP_RETURN( _TXT("error in 'punctuation' tokenizer: %s"), *m_errorhnd, 0);
	}

	virtual const char* getDescription() const
	{
		return _TXT("Tokenizer producing punctuation elements (end of sentence recognition). The language is specified as parameter (currently only german 'de' and english 'en' supported)");
	}
	
private:
	ErrorBufferInterface* m_errorhnd;
};

static bool g_intl_initialized = false;

TokenizerFunctionInterface* strus::punctuationTokenizer( ErrorBufferInterface* errorhnd)
{
	try
	{
		if (!g_intl_initialized)
		{
			strus::initMessageTextDomain();
			g_intl_initialized = true;
		}
		return new PunctuationTokenizerFunction( errorhnd);
	}
	CATCH_ERROR_MAP_RETURN( _TXT("cannot create punctuation tokenizer: %s"), *errorhnd, 0);
}


