/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "strus/lib/tokenizer_word.hpp"
#include "strus/tokenizerFunctionInterface.hpp"
#include "strus/tokenizerFunctionInstanceInterface.hpp"
#include "strus/errorBufferInterface.hpp"
#include "strus/analyzer/token.hpp"
#include "strus/analyzer/functionView.hpp"
#include "strus/base/dll_tags.hpp"
#include "private/errorUtils.hpp"
#include "private/internationalization.hpp"
#include "private/unicodeWordDelimiters.hpp"
#include "private/tokenizeHelpers.hpp"
#include <vector>
#include <string>

using namespace strus;
using namespace strus::analyzer;

typedef bool (*TokenDelimiterFunction)( char const* si, const char* se);

struct TokenDelimiter
{
	const char* name;
	TokenDelimiterFunction func;
};

class SeparationTokenizerInstance
	:public TokenizerFunctionInstanceInterface
{
public:
	SeparationTokenizerInstance( const TokenDelimiter* delim, ErrorBufferInterface* errorhnd)
		:m_delim(delim),m_errorhnd(errorhnd){}

	const char* skipToToken( char const* si, const char* se) const;

	virtual std::vector<Token> tokenize( const char* src, std::size_t srcsize) const;

	virtual bool concatBeforeTokenize() const
	{
		return false;
	}

	virtual analyzer::FunctionView view() const
	{
		try
		{
			return analyzer::FunctionView( m_delim->name);
		}
		CATCH_ERROR_MAP_RETURN( _TXT("error in introspection: %s"), *m_errorhnd, analyzer::FunctionView());
	}

private:
	const TokenDelimiter* m_delim;
	ErrorBufferInterface* m_errorhnd;
};

class SeparationTokenizerFunction
	:public TokenizerFunctionInterface
{
public:
	SeparationTokenizerFunction( const char* description_, const TokenDelimiter* delim_, ErrorBufferInterface* errorhnd_)
		:m_delim(delim_),m_description(description_),m_errorhnd(errorhnd_){}

	TokenizerFunctionInstanceInterface* createInstance( const std::vector<std::string>& args, const TextProcessorInterface*) const
	{
		if (args.size())
		{
			m_errorhnd->report( ErrorCodeInvalidArgument, _TXT("no arguments expected for tokenizer"));
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
	const TokenDelimiter* m_delim;
	const char* m_description;
	ErrorBufferInterface* m_errorhnd;
};



const char* SeparationTokenizerInstance::skipToToken( char const* si, const char* se) const
{
	for (; si < se && m_delim->func( si, se); si = skipChar( si)){}
	return si;
}

std::vector<Token> SeparationTokenizerInstance::tokenize( const char* src, std::size_t srcsize) const
{
	try
	{
		std::vector<Token> rt;
		char const* si = skipToToken( src, src+srcsize);
		const char* se = src+srcsize;
	
		for (;si < se; si = skipToToken(si,se))
		{
			const char* start = si;
			while (si < se && !m_delim->func( si, se))
			{
				si = skipChar( si);
			}
			rt.push_back( Token( start-src, 0/*seg*/, start-src, si-start));
		}
		return rt;
	}
	CATCH_ERROR_MAP_RETURN( _TXT("error in tokenizer: %s"), *m_errorhnd, std::vector<Token>());
}


static bool g_intl_initialized = false;

DLL_PUBLIC TokenizerFunctionInterface* strus::createTokenizer_word( ErrorBufferInterface* errorhnd)
{
	try
	{
		static const TokenDelimiter delim = {"word", &wordBoundaryDelimiter};
		if (!g_intl_initialized)
		{
			strus::initMessageTextDomain();
			g_intl_initialized = true;
		}
		return new SeparationTokenizerFunction( _TXT("Tokenizer splitting tokens by word boundaries"), &delim, errorhnd);
	}
	CATCH_ERROR_MAP_RETURN( _TXT("cannot create word tokenizer: %s"), *errorhnd, 0);
}

DLL_PUBLIC TokenizerFunctionInterface* strus::createTokenizer_whitespace( ErrorBufferInterface* errorhnd)
{
	try
	{
		static const TokenDelimiter delim = {"split", &whiteSpaceDelimiter};
		if (!g_intl_initialized)
		{
			strus::initMessageTextDomain();
			g_intl_initialized = true;
		}
		return new SeparationTokenizerFunction( _TXT( "Tokenizer splitting tokens separated by whitespace characters"), &delim, errorhnd);
	}
	CATCH_ERROR_MAP_RETURN( _TXT("cannot create whitespace tokenizer: %s"), *errorhnd, 0);
}



