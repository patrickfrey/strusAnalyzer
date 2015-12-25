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
#include "strus/lib/tokenizer_word.hpp"
#include "strus/tokenizerFunctionInterface.hpp"
#include "strus/tokenizerFunctionInstanceInterface.hpp"
#include "strus/tokenizerFunctionContextInterface.hpp"
#include "strus/analyzerErrorBufferInterface.hpp"
#include "strus/analyzer/token.hpp"
#include "private/dll_tags.hpp"
#include "private/errorUtils.hpp"
#include "private/internationalization.hpp"
#include "textwolf/charset_utf8.hpp"
#include <vector>
#include <string>

using namespace strus;
using namespace strus::analyzer;

typedef bool (*TokenDelimiter)( char const* si, const char* se);

class SeparationTokenizerFunctionContext
	:public TokenizerFunctionContextInterface
{
public:
	SeparationTokenizerFunctionContext( TokenDelimiter delim, AnalyzerErrorBufferInterface* errorhnd)
		:m_delim(delim),m_errorhnd(errorhnd){}

	const char* skipToToken( char const* si, const char* se) const;

	virtual std::vector<Token> tokenize( const char* src, std::size_t srcsize);

private:
	TokenDelimiter m_delim;
	AnalyzerErrorBufferInterface* m_errorhnd;
};


class SeparationTokenizerInstance
	:public TokenizerFunctionInstanceInterface
{
public:
	SeparationTokenizerInstance( TokenDelimiter delim, AnalyzerErrorBufferInterface* errorhnd)
		:m_delim(delim),m_errorhnd(errorhnd){}

	TokenizerFunctionContextInterface* createFunctionContext() const
	{
		try
		{
			return new SeparationTokenizerFunctionContext( m_delim, m_errorhnd);
		}
		CATCH_ERROR_MAP_RETURN( _TXT("cannot create tokenizer: %s"), *m_errorhnd, 0);
	}

private:
	TokenDelimiter m_delim;
	AnalyzerErrorBufferInterface* m_errorhnd;
};

class SeparationTokenizerFunction
	:public TokenizerFunctionInterface
{
public:
	SeparationTokenizerFunction( const char* description_, TokenDelimiter delim_, AnalyzerErrorBufferInterface* errorhnd_)
		:m_delim(delim_),m_description(description_),m_errorhnd(errorhnd_){}

	TokenizerFunctionInstanceInterface* createInstance( const std::vector<std::string>& args, const TextProcessorInterface*) const
	{
		if (args.size())
		{
			m_errorhnd->report( "no arguments expected for tokenizer");
			return 0;
		}
		try
		{
			return new SeparationTokenizerInstance( m_delim, m_errorhnd);
		}
		CATCH_ERROR_MAP_RETURN( _TXT("error in tokenizer: %s"), *m_errorhnd, 0);
	}

	virtual const char* getDescription() const
	{
		return m_description;
	}

private:
	TokenDelimiter m_delim;
	const char* m_description;
	AnalyzerErrorBufferInterface* m_errorhnd;
};


static textwolf::charset::UTF8::CharLengthTab g_charLengthTab;

static inline const char* skipChar( const char* si)
{
	return si+g_charLengthTab[*si];
}

static inline unsigned int utf8decode( char const* si, const char* se)
{
	enum {
		B00111111=0x3F,
		B00011111=0x1F
	};
	unsigned int res = (unsigned char)*si;
	unsigned char charsize = g_charLengthTab[ *si];
	if (res > 127)
	{
		res = ((unsigned char)*si)&(B00011111>>(charsize-2));
		for (++si,--charsize; si != se && charsize; ++si,--charsize)
		{
			res <<= 6;
			res |= (unsigned char)(*si & B00111111);
		}
	}
	return res;
}

class CharTable
{
public:
	CharTable( const char* op)
	{
		std::size_t ii;
		for (ii=0; ii<sizeof(m_ar); ++ii) m_ar[ii] = false;
		for (ii=0; op[ii]; ++ii)
		{
			m_ar[(unsigned char)(op[ii])] = true;
		}
	}

	bool operator[]( char ch) const
	{
		return m_ar[ (unsigned char)ch];
	}

private:
	bool m_ar[128];
};


static bool wordBoundaryDelimiter( char const* si, const char* se)
{
	static const CharTable wordCharacter("0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ");
	if ((unsigned char)*si <= 32)
	{
		return true;
	}
	else if ((unsigned char)*si >= 128)
	{
		unsigned int chr = utf8decode( si, se);
		if (chr == 133) return true;
		if (chr >= 0x2000 && chr <= 0x206F) return true;
		if (chr == 0x3000) return true;
		if (chr == 0xFEFF) return true;
		return false;
	}
	else if (wordCharacter[ *si])
	{
		return false;
	}
	else
	{
		return true;
	}
}

static bool whiteSpaceDelimiter( char const* si, const char* se)
{
	if ((unsigned char)*si <= 32)
	{
		return true;
	}
	else if ((unsigned char)*si >= 128)
	{
		unsigned int chr = utf8decode( si, se);
		if (chr == 133) return true;
		if (chr >= 0x2000 && chr <= 0x200F) return true;
		if (chr >= 0x2028 && chr <= 0x2029) return true;
		if (chr == 0x202F) return true;
		if (chr >= 0x205F && chr <= 0x2060) return true;
		if (chr == 0x3000) return true;
		if (chr == 0xFEFF) return true;
		return false;
	}
	else
	{
		return false;
	}
}

const char* SeparationTokenizerFunctionContext::skipToToken( char const* si, const char* se) const
{
	for (; si < se && m_delim( si, se); si = skipChar( si)){}
	return si;
}

std::vector<Token> SeparationTokenizerFunctionContext::tokenize( const char* src, std::size_t srcsize)
{
	try
	{
		std::vector<Token> rt;
		char const* si = skipToToken( src, src+srcsize);
		const char* se = src+srcsize;
	
		for (;si < se; si = skipToToken(si,se))
		{
			const char* start = si;
			while (si < se && !m_delim( si, se))
			{
				si = skipChar( si);
			}
			rt.push_back( Token( start-src, start-src, si-start));
		}
		return rt;
	}
	CATCH_ERROR_MAP_RETURN( _TXT("error in tokenizer: %s"), *m_errorhnd, std::vector<Token>());
}


static bool g_intl_initialized = false;

DLL_PUBLIC TokenizerFunctionInterface* strus::createTokenizer_word( AnalyzerErrorBufferInterface* errorhnd)
{
	try
	{
		if (!g_intl_initialized)
		{
			strus::initMessageTextDomain();
			g_intl_initialized = true;
		}
		return new SeparationTokenizerFunction( _TXT("Tokenizer splitting tokens by word boundaries for european languages"), wordBoundaryDelimiter, errorhnd);
	}
	CATCH_ERROR_MAP_RETURN( _TXT("cannot create word tokenizer: %s"), *errorhnd, 0);
}

DLL_PUBLIC TokenizerFunctionInterface* strus::createTokenizer_whitespace( AnalyzerErrorBufferInterface* errorhnd)
{
	try
	{
		if (!g_intl_initialized)
		{
			strus::initMessageTextDomain();
			g_intl_initialized = true;
		}
		return new SeparationTokenizerFunction( _TXT( "Tokenizer splitting tokens separated by whitespace characters"), whiteSpaceDelimiter, errorhnd);
	}
	CATCH_ERROR_MAP_RETURN( _TXT("cannot create whitespace tokenizer: %s"), *errorhnd, 0);
}



