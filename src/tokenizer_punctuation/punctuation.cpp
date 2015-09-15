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
#include "punctuation.hpp"
#include "strus/tokenizerFunctionInterface.hpp"
#include "strus/tokenizerFunctionInstanceInterface.hpp"
#include "strus/tokenizerFunctionContextInterface.hpp"
#include "strus/analyzerErrorBufferInterface.hpp"
#include "private/utils.hpp"
#include "private/errorUtils.hpp"
#include "private/internationalization.hpp"
#include "punctuation_de.hpp"
#include "punctuation_en.hpp"
#include <cstring>
#include <iostream>
#include <stdexcept>

using namespace strus;
using namespace strus::analyzer;

#undef STRUS_LOWLEVEL_DEBUG

class PunctuationTokenizerFunction
	:public TokenizerFunctionInterface
{
public:
	virtual TokenizerFunctionInstanceInterface* createInstance( const std::vector<std::string>& args, const TextProcessorInterface*, AnalyzerErrorBufferInterface* errorhnd) const
	{
		try
		{
			if (args.size() > 2)
			{
				errorhnd->report( _TXT("too many arguments for punctuation tokenizer (1st language 2nd optional punctuation characters)"));
				return 0;
			}
			if (args.size() < 1)
			{
				errorhnd->report( _TXT("too few arguments for punctuation tokenizer (language as mandatory argument expected)"));
				return 0;
			}
			const char* punctChar = 0;
			if (args.size() == 2)
			{
				punctChar = args[1].c_str();
			}
			if (utils::caseInsensitiveEquals( args[0], "de"))
			{
				return new PunctuationTokenizerInstance_de( punctChar, errorhnd);
			}
			else if (utils::caseInsensitiveEquals( args[0], "en"))
			{
				return new PunctuationTokenizerInstance_en( punctChar, errorhnd);
			}
			else
			{
				errorhnd->report( _TXT("unsupported language passed to punctuation tokenizer ('%s')"), args[0].c_str());
				return 0;
			}
		}
		CATCH_ERROR_MAP_RETURN( _TXT("error in 'punctuation' tokenizer: %s"), *errorhnd, 0);
	}
};

const TokenizerFunctionInterface* strus::punctuationTokenizer()
{
	static const PunctuationTokenizerFunction tokenizer;
	return &tokenizer;
}


