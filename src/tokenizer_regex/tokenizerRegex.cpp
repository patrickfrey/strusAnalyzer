/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "tokenizerRegex.hpp"
#include "strus/errorBufferInterface.hpp"
#include "strus/tokenizerFunctionContextInterface.hpp"
#include "strus/tokenizerFunctionInstanceInterface.hpp"
#include "private/utils.hpp"
#include "private/errorUtils.hpp"
#include "private/internationalization.hpp"
#include <cstring>
#include <boost/regex.hpp>

using namespace strus;

struct RegexConfiguration
{
	boost::regex expression;

	explicit RegexConfiguration( const std::string& expressionstr)
		:expression(expressionstr){}
	RegexConfiguration( const RegexConfiguration& o)
		:expression(o.expression){}
};

class RegexTokenizerFunctionContext
	:public TokenizerFunctionContextInterface
{
public:
	RegexTokenizerFunctionContext( const RegexConfiguration& config_, ErrorBufferInterface* errorhnd_)
		:m_errorhnd(errorhnd_),m_config(config_){}

	virtual ~RegexTokenizerFunctionContext(){}

	virtual std::vector<analyzer::Token> tokenize( const char* src, std::size_t srcsize)
	{
		try
		{
			std::vector<analyzer::Token> rt;
			boost::match_results<char const*> what;
			char const* si = src;
			char const* se = src+srcsize;
			while (boost::regex_search( si, se, what, m_config.expression, boost::match_posix))
			{
				std::size_t len = what.length();
				std::size_t pos = what.position();
				rt.push_back( analyzer::Token( pos, pos, len));
				si += pos + len;
			}
			return rt;
		}
		CATCH_ERROR_MAP_RETURN( _TXT("error executing \"regex\" tokenizer function: %s"), *m_errorhnd, std::vector<analyzer::Token>());
	}

private:
	ErrorBufferInterface* m_errorhnd;
	RegexConfiguration m_config;
};


class RegexTokenizerFunctionInstance
	:public TokenizerFunctionInstanceInterface
{
public:
	RegexTokenizerFunctionInstance( const RegexConfiguration& config_, ErrorBufferInterface* errorhnd_)
		:m_errorhnd(errorhnd_),m_config(config_){}

	virtual ~RegexTokenizerFunctionInstance(){}

	virtual bool concatBeforeTokenize() const
	{
		return false;
	}

	virtual TokenizerFunctionContextInterface* createFunctionContext() const
	{
		try
		{
			return new RegexTokenizerFunctionContext( m_config, m_errorhnd);
		}
		CATCH_ERROR_MAP_RETURN( _TXT("error creating \"regex\" tokenizer function context: %s"), *m_errorhnd, 0);
	}

private:
	ErrorBufferInterface* m_errorhnd;
	RegexConfiguration m_config;
};


TokenizerFunctionInstanceInterface* RegexTokenizerFunction::createInstance(
			const std::vector<std::string>& args,
			const TextProcessorInterface* tp) const
{
	try
	{
		if (args.size() > 1)
		{
			throw strus::runtime_error(_TXT("too many arguments for \"regex\" tokenizer (one argument, the regular expression to find tokens)"));
		}
		else if (args.size() < 1)
		{
			throw strus::runtime_error(_TXT("expected argument for \"regex\" tokenizer: the regular expression to find tokens)"));
		}
		return new RegexTokenizerFunctionInstance( RegexConfiguration( args[0]), m_errorhnd);
	}
	CATCH_ERROR_MAP_RETURN( _TXT("error creating \"regex\" tokenizer instance: %s"), *m_errorhnd, 0);
}

const char* RegexTokenizerFunction::getDescription() const
{
	try
	{
		return _TXT("Tokenizer selecting tokens from source that are matching a regular expression.");
	}
	CATCH_ERROR_MAP_RETURN( _TXT("error getting \"regex\" tokenizer description: %s"), *m_errorhnd, 0);
}


