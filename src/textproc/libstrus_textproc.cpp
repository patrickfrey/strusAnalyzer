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
#include "textProcessor.hpp"
#include "private/dll_tags.hpp"
#include <stdexcept>

using namespace strus;

DLL_PUBLIC strus::TextProcessorInterface*
	strus::createTextProcessor()
{
	TextProcessor* rt = new TextProcessor();
	try
	{
		rt->defineNormalizer( "stem", getNormalizer_snowball());
		rt->defineNormalizer( "dictmap", getNormalizer_dictmap());
		rt->defineNormalizer( "lc", getNormalizer_lowercase());
		rt->defineNormalizer( "uc", getNormalizer_uppercase());
		rt->defineNormalizer( "convdia", getNormalizer_convdia());
		rt->defineNormalizer( "date2int", getNormalizer_date2int());
		rt->defineTokenizer( "punctuation", getTokenizer_punctuation());
		rt->defineTokenizer( "word", getTokenizer_word());
		rt->defineTokenizer( "split", getTokenizer_whitespace());
		return rt;
	}
	catch (const std::runtime_error& err)
	{
		delete rt;
		throw err;
	}
	catch (const std::bad_alloc& err)
	{
		delete rt;
		throw err;
	}
}


