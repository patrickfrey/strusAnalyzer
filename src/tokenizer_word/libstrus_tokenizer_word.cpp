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

	virtual const char* name() const	{return m_delim->name;}
	virtual StructView view() const
	{
		try
		{
			return StructView()( "name", m_delim->name);
		}
		CATCH_ERROR_MAP_RETURN( _TXT("error in introspection: %s"), *m_errorhnd, StructView());
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
		:m_description(description_),m_delim(delim_),m_errorhnd(errorhnd_){}

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

	virtual const char* name() const	{return m_delim->name;}
	virtual StructView view() const
	{
		try
		{
			return StructView()
				("name", name())
				("description", m_description)
			;
		}
		CATCH_ERROR_MAP_RETURN( _TXT("error in introspection: %s"), *m_errorhnd, StructView());
	}

private:
	const char* m_description;
	const TokenDelimiter* m_delim;
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
			rt.push_back( Token( start-src, analyzer::Position(0/*seg*/, start-src), si-start));
		}
		return rt;
	}
	CATCH_ERROR_MAP_RETURN( _TXT("error in tokenizer: %s"), *m_errorhnd, std::vector<Token>());
}


class LangTokenTokenizerInstance
	:public TokenizerFunctionInstanceInterface
{
public:
	explicit LangTokenTokenizerInstance( ErrorBufferInterface* errorhnd)
		:m_errorhnd(errorhnd){}

	virtual std::vector<Token> tokenize( const char* src, std::size_t srcsize) const
	{
		std::vector<Token> rt;
		int pos = 0;
		int ordpos = 0;
		SourceSpan item;
		while ((item=getNextPosTaggingEntity( src, srcsize, pos)).defined())
		{
			rt.push_back( Token( ++ordpos, analyzer::Position( 0/*seg*/, item.pos), item.len));
		}
		return rt;
	}

	virtual bool concatBeforeTokenize() const
	{
		return false;
	}

	virtual const char* name() const	{return "langtoken";}
	virtual StructView view() const
	{
		try
		{
			return StructView()( "name", name());
		}
		CATCH_ERROR_MAP_RETURN( _TXT("error in introspection: %s"), *m_errorhnd, StructView());
	}

private:
	ErrorBufferInterface* m_errorhnd;
};

class LangTokenTokenizerFunction
	:public TokenizerFunctionInterface
{
public:
	explicit LangTokenTokenizerFunction( ErrorBufferInterface* errorhnd_)
		:m_errorhnd(errorhnd_){}

	TokenizerFunctionInstanceInterface* createInstance( const std::vector<std::string>& args, const TextProcessorInterface*) const
	{
		if (args.size())
		{
			m_errorhnd->report( ErrorCodeInvalidArgument, _TXT("no arguments expected for tokenizer"));
			return 0;
		}
		try
		{
			return new LangTokenTokenizerInstance( m_errorhnd);
		}
		CATCH_ERROR_ARG1_MAP_RETURN( _TXT("error in '%s' tokenizer: %s"), "langtoken", *m_errorhnd, 0);
	}

	virtual const char* name() const	{return "langtoken";}
	virtual StructView view() const
	{
		try
		{
			return StructView()
				("name", name())
				("description", _TXT("Tokenizer returning all sequences of alphanumeric characters as words and word boundary delimiters as single characters"))
			;
		}
		CATCH_ERROR_MAP_RETURN( _TXT("error in introspection: %s"), *m_errorhnd, StructView());
	}

private:
	ErrorBufferInterface* m_errorhnd;
};


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
	CATCH_ERROR_ARG1_MAP_RETURN( _TXT("cannot create '%s' tokenizer: %s"), "word", *errorhnd, 0);
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
	CATCH_ERROR_ARG1_MAP_RETURN( _TXT("cannot create '%s' tokenizer: %s"), "split", *errorhnd, 0);
}

DLL_PUBLIC TokenizerFunctionInterface* strus::createTokenizer_langtoken( ErrorBufferInterface* errorhnd)
{
	try
	{
		if (!g_intl_initialized)
		{
			strus::initMessageTextDomain();
			g_intl_initialized = true;
		}
		return new LangTokenTokenizerFunction( errorhnd);
	}
	CATCH_ERROR_ARG1_MAP_RETURN( _TXT("cannot create '%s' tokenizer: %s"), "langtoken", *errorhnd, 0);
}



