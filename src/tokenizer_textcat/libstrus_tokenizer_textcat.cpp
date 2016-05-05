/*
 * Copyright (c) 2016 Andreas Baumann <mail@andreasbaumann.cc>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "strus/lib/tokenizer_textcat.hpp"
#include "strus/tokenizerFunctionInterface.hpp"
#include "strus/tokenizerFunctionInstanceInterface.hpp"
#include "strus/tokenizerFunctionContextInterface.hpp"
#include "strus/errorBufferInterface.hpp"
#include "strus/analyzer/token.hpp"
#include "strus/base/dll_tags.hpp"
#include "private/errorUtils.hpp"
#include "private/internationalization.hpp"
extern "C" {
#include "textcat.h"
}

using namespace strus;
using namespace strus::analyzer;

#include <iostream>
#include <stdexcept>
#include <unistd.h>
#include <libgen.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>

#include "textwolf/charset_utf8.hpp"

class TextcatTokenizerFunctionContext
	:public TokenizerFunctionContextInterface
{
public:
	TextcatTokenizerFunctionContext( const std::string& config, const std::string& language, ErrorBufferInterface* errorhnd)
		:m_language(language), m_errorhnd(errorhnd), m_textcat(0)
	{
		char* oldDir = new char[PATH_MAX];
		char* configCopy = strdup( config.c_str());
		char* dir = dirname( configCopy);
		(void)chdir( dir);
		free( configCopy);
		
		m_textcat = textcat_Init( config.c_str());
		(void)chdir( oldDir);
		if( !m_textcat) {
			throw new std::runtime_error( "Cannot open textcat configuration");
		}
	}
	
	virtual ~TextcatTokenizerFunctionContext() {
		if( m_textcat) {
			textcat_Done(m_textcat);
		}
	}

	const char* skipToToken( char const* si, const char* se) const;

	virtual std::vector<Token> tokenize( const char* src, std::size_t srcsize);

private:
	std::string m_language;
	ErrorBufferInterface* m_errorhnd;
	void *m_textcat;
};

static textwolf::charset::UTF8::CharLengthTab g_charLengthTab;

static inline const char* skipChar( const char* si)
{
	return si+g_charLengthTab[*si];
}

const char* TextcatTokenizerFunctionContext::skipToToken( char const* si, const char* se) const
{
	for (; si < se && isspace( *si); si = skipChar( si)){}
	return si;
}

std::vector<Token> TextcatTokenizerFunctionContext::tokenize( const char* src, std::size_t srcsize)
{
	try
	{
		std::vector<Token> rt;

		char *languages;

		languages = textcat_Classify( m_textcat, const_cast<char *>( src ), srcsize );
std::cout << "languages: " << languages << " (must: " << m_language << ")" << std::endl;		
		if( strcmp( languages, _TEXTCAT_RESULT_UNKOWN ) == 0 ) {
			m_errorhnd->report( _TXT("unknown languages seen in textcat in: %*s"), srcsize, src);
		} else if( strcmp( languages, _TEXTCAT_RESULT_SHORT ) == 0 ) {
			m_errorhnd->report( _TXT("text too short in textcat in: %*s"), srcsize, src);
		} else if( strstr( m_language.c_str( ), languages ) == NULL ) {
			// not the language we are lookup for, so do not emit tokens
			return rt;
		}

		char const* si = skipToToken( src, src+srcsize);
		const char* se = src+srcsize;
	
		for (;si < se; si = skipToToken(si,se))
		{
			const char* start = si;
			while (si < se && !isspace( *si))
			{
				si = skipChar( si);
			}
			rt.push_back( Token( start-src, start-src, si-start));
		}
		
		return rt;
	}
	CATCH_ERROR_MAP_RETURN( _TXT("error in tokenizer: %s"), *m_errorhnd, std::vector<Token>());

}

class TextcatTokenizerInstance
	:public TokenizerFunctionInstanceInterface
{
public:
	TextcatTokenizerInstance( const std::string& config, const std::string& language, ErrorBufferInterface* errorhnd)
		:m_config(config), m_language(language), m_errorhnd(errorhnd){}

	TokenizerFunctionContextInterface* createFunctionContext() const
	{
		try
		{
			return new TextcatTokenizerFunctionContext( m_config, m_language, m_errorhnd);
		}
		CATCH_ERROR_MAP_RETURN( _TXT("cannot create tokenizer: %s"), *m_errorhnd, 0);
	}

	virtual bool concatBeforeTokenize() const
	{
		return true;
	}

private:
	std::string m_config;
	std::string m_language;
	ErrorBufferInterface* m_errorhnd;
};

class TextcatTokenizerFunction
	:public TokenizerFunctionInterface
{
public:
	TextcatTokenizerFunction( const char* description_, ErrorBufferInterface* errorhnd_)
		:m_description(description_),m_errorhnd(errorhnd_){}

	TokenizerFunctionInstanceInterface* createInstance( const std::vector<std::string>& args, const TextProcessorInterface*) const
	{
		if (args.size() == 0)
		{
			m_errorhnd->report( _TXT("name of textcat config file expected as first argument for the 'textcat' tokenizer"));
			return 0;
		}
		if (args.size() > 2)
		{
			m_errorhnd->report( _TXT("too many arguments for 'textcat' tokenizer"));
			return 0;
		}
		try
		{
			return new TextcatTokenizerInstance( args[0], args[1], m_errorhnd);
		}
		CATCH_ERROR_MAP_RETURN( _TXT("error in tokenizer: %s"), *m_errorhnd, 0);
	}

	virtual const char* getDescription() const
	{
		return m_description;
	}

private:
	const char* m_description;
	ErrorBufferInterface* m_errorhnd;
};

static bool g_intl_initialized = false;

DLL_PUBLIC TokenizerFunctionInterface* strus::createTokenizer_textcat( ErrorBufferInterface* errorhnd)
{
	try
	{
		if (!g_intl_initialized)
		{
			strus::initMessageTextDomain();
			g_intl_initialized = true;
		}
		return new TextcatTokenizerFunction( _TXT("Tokenizer splitting tokens by recognized language"), errorhnd);
	}
	CATCH_ERROR_MAP_RETURN( _TXT("cannot create textcat tokenizer: %s"), *errorhnd, 0);
}
