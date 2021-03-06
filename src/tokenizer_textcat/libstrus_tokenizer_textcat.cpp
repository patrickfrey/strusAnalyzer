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
#include "strus/errorBufferInterface.hpp"
#include "strus/textProcessorInterface.hpp"
#include "strus/analyzer/token.hpp"
#include "strus/base/dll_tags.hpp"
#include "private/errorUtils.hpp"
#include "private/internationalization.hpp"
extern "C" {
#include "textcat.h"
}

using namespace strus;
using namespace strus::analyzer;

#include <stdexcept>
#include <unistd.h>
#include <libgen.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>

#include "textwolf/charset_utf8.hpp"

#undef STRUS_LOWLEVEL_DEBUG

#ifdef STRUS_LOWLEVEL_DEBUG
#include <iostream>
#endif


class TextcatTokenizerInstance
	:public TokenizerFunctionInstanceInterface
{
public:
	TextcatTokenizerInstance( const std::string& config, const std::string& language, ErrorBufferInterface* errorhnd)
		:m_language(language), m_errorhnd(errorhnd), m_textcat(0)
	{
		initialize( config );
	}

	virtual ~TextcatTokenizerInstance() {
		if (m_textcat) {
			textcat_Done(m_textcat);
		}
	}

	const char* skipToToken( char const* si, const char* se) const;

	std::vector<Token> tokenize( const char* src, std::size_t srcsize) const;

	virtual bool concatBeforeTokenize() const
	{
		return true;
	}

	virtual const char* name() const	{return "textcat";}
	virtual StructView view() const
	{
		try
		{
			return StructView()
				("name", name())
				("language", m_language)
			;
		}
		CATCH_ERROR_MAP_RETURN( _TXT("error in introspection: %s"), *m_errorhnd, StructView());
	}

private:
	std::string m_language;
	ErrorBufferInterface* m_errorhnd;
	void *m_textcat;

	void initialize( const std::string &config )
	{
		char* oldDir = new char[PATH_MAX];
		if( getcwd( oldDir, PATH_MAX ) == NULL) {
			delete[] oldDir;
			throw strus::runtime_error( _TXT("cannot get current working directory"));
		}
		char* configCopy = strdup( config.c_str());
		if( configCopy == NULL) {
			delete[] oldDir;
			throw strus::runtime_error( _TXT("failed to allocate memory for name of textcat configuration file"));
		}
		char* dir = dirname( configCopy);
		if( chdir( dir) < 0) {
			free( configCopy);
			delete[] oldDir;
			throw strus::runtime_error( _TXT("cannot change to directory of textcat configuration file"));
		}
		free( configCopy);

		m_textcat = textcat_Init( config.c_str());
		if( chdir( oldDir) < 0) {
			delete[] oldDir;
			throw strus::runtime_error( _TXT("cannot change to original directory"));
		}
		delete[] oldDir;
		if( !m_textcat) {
			throw strus::runtime_error( _TXT("cannot open textcat configuration"));
		}
	}
};

static textwolf::charset::UTF8::CharLengthTab g_charLengthTab;

static inline const char* skipChar( const char* si)
{
	unsigned char charsize = g_charLengthTab[ *si];
	if (!charsize)
	{
		throw strus::runtime_error(_TXT( "illegal UTF-8 character in input: %u"), (unsigned int)(unsigned char)*si);
	}
	else
	{
		return si+charsize;
	}
}

const char* TextcatTokenizerInstance::skipToToken( char const* si, const char* se) const
{
	for (; si < se && isspace( *si); si = skipChar( si)){}
	return si;
}

std::vector<Token> TextcatTokenizerInstance::tokenize( const char* src, std::size_t srcsize) const
{
	try
	{
		std::vector<Token> rt;

		char *languages;

		languages = textcat_Classify( m_textcat, const_cast<char *>( src ), srcsize );
		if( strcmp( languages, _TEXTCAT_RESULT_UNKOWN ) == 0 ) {
			// unknown languages seen, we assume some other rule catches
			// the non-recognizable things in the text and adds them somehow
#ifdef STRUS_LOWLEVEL_DEBUG
			std::cout << "textcat: unknown language" << std::endl;
#endif
			return rt;
		} else if( strcmp( languages, _TEXTCAT_RESULT_SHORT ) == 0 ) {
			// text too short in textcat
			// TODO: can I issue warnings in error handler?
#ifdef STRUS_LOWLEVEL_DEBUG
			std::cout << "textcat: short text" << std::endl;
#endif
		} else if( strstr( languages, m_language.c_str( ) ) == NULL ) {
			// not the language we are lookup for, so do not emit tokens
#ifdef STRUS_LOWLEVEL_DEBUG
			std::cout << "textcat: detected languages don't match '" << m_language << "'"
				<< "(" << languages << ")" << std::endl;
#endif
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
			Token token( start-src/*ord*/, analyzer::Position(0/*seg*/, start-src), si-start);
			rt.push_back( token);
#ifdef STRUS_LOWLEVEL_DEBUG
			std::cout << "textcat: " << m_language << " "
				<< token.strpos << " " << token.strsize << " "
				<< std::string( start, si-start)
				<< std::endl;
#endif
		}
		
		return rt;
	}
	CATCH_ERROR_MAP_RETURN( _TXT("error in tokenizer: %s"), *m_errorhnd, std::vector<Token>());

}

class TextcatTokenizerFunction
	:public TokenizerFunctionInterface
{
public:
	TextcatTokenizerFunction( const TextProcessorInterface* textproc_, ErrorBufferInterface* errorhnd_)
		:m_textproc(textproc_),m_errorhnd(errorhnd_){}

	TokenizerFunctionInstanceInterface* createInstance( const std::vector<std::string>& args, const TextProcessorInterface*) const
	{
		if (args.size() == 0)
		{
			m_errorhnd->report(  ErrorCodeIncompleteDefinition, _TXT("name of textcat config file expected as first argument for the 'textcat' tokenizer"));
			return 0;
		}
		if (args.size() == 1)
		{
			m_errorhnd->report( ErrorCodeIncompleteDefinition, _TXT("filter language expected as second parameter of the textcat tokenizer"));
			return 0;
		}
		if (args.size() > 2)
		{
			m_errorhnd->report( ErrorCodeInvalidArgument, _TXT("too many arguments for 'textcat' tokenizer"));
			return 0;
		}
		try
		{
			std::string resolvedConfigFilename = m_textproc->getResourceFilePath( args[0]);
			if (resolvedConfigFilename.empty() && m_errorhnd->hasError())
			{
				throw strus::runtime_error(_TXT("could not resolve path of file '%s': %s"), args[0].c_str(), m_errorhnd->fetchError());
			}
			return new TextcatTokenizerInstance( resolvedConfigFilename, args[1]/*language*/, m_errorhnd);
		}
		CATCH_ERROR_MAP_RETURN( _TXT("error in tokenizer: %s"), *m_errorhnd, 0);
	}

	virtual const char* name() const	{return "textcat";}
	virtual StructView view() const
	{
		try
		{
			return StructView()
				("name", name())
				("description", _TXT("Tokenizer splitting tokens by recognized language"))
			;
		}
		CATCH_ERROR_MAP_RETURN( _TXT("error in introspection: %s"), *m_errorhnd, StructView());
	}

private:
	const TextProcessorInterface* m_textproc;
	ErrorBufferInterface* m_errorhnd;
};

static bool g_intl_initialized = false;

DLL_PUBLIC TokenizerFunctionInterface* strus::createTokenizer_textcat( const TextProcessorInterface* textproc, ErrorBufferInterface* errorhnd)
{
	try
	{
		if (!g_intl_initialized)
		{
			strus::initMessageTextDomain();
			g_intl_initialized = true;
		}
		return new TextcatTokenizerFunction( textproc, errorhnd);
	}
	CATCH_ERROR_MAP_RETURN( _TXT("cannot create textcat tokenizer: %s"), *errorhnd, 0);
}

