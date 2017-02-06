/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "tokenizerRegex.hpp"
#include "strus/errorBufferInterface.hpp"
#include "strus/tokenizerFunctionInstanceInterface.hpp"
#include "private/utils.hpp"
#include "private/errorUtils.hpp"
#include "private/internationalization.hpp"
#include <cstring>
#include <boost/regex.hpp>

using namespace strus;

#define TOKENIZER_NAME "regex"

struct RegexConfiguration
{
	boost::regex expression;
	unsigned int index;

	RegexConfiguration( const std::string& expressionstr, unsigned int index_)
		:expression(expressionstr),index(index_){}
	RegexConfiguration( const RegexConfiguration& o)
		:expression(o.expression),index(o.index){}
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

	virtual std::vector<analyzer::Token> tokenize( const char* src, std::size_t srcsize) const
	{
		try
		{
			std::vector<analyzer::Token> rt;
			boost::match_results<char const*> what;
			char const* si = src;
			char const* se = src+srcsize;
			while (si < se
				&& boost::regex_search(
					si, se, what, m_config.expression,
					boost::match_posix))
			{
				std::size_t len = what.length( m_config.index);
				std::size_t pos = what.position( m_config.index);
				std::size_t abspos = pos + (si - src);
				rt.push_back( analyzer::Token( abspos/*ord*/, 0/*seg*/, abspos, len));
				if (pos + len == 0)
				{
					++si;
				}
				else
				{
					si += pos + len;
				}
			}
			return rt;
		}
		CATCH_ERROR_MAP_ARG1_RETURN( _TXT("error executing \"%s\" tokenizer function: %s"), TOKENIZER_NAME, *m_errorhnd, std::vector<analyzer::Token>());
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
		if (args.size() > 2)
		{
			throw strus::runtime_error(_TXT("too many arguments for \"%s\" tokenizer (maximum two arguments, the regular expression to find tokens and an opional selector index)"), TOKENIZER_NAME);
		}
		else if (args.size() < 1)
		{
			throw strus::runtime_error(_TXT("expected argument for \"%s\" tokenizer: the regular expression to find tokens)"), TOKENIZER_NAME);
		}
		unsigned int selectIndex = 0;
		if (args.size() == 2)
		{
			selectIndex = utils::touint( args[1]);
		}
		return new RegexTokenizerFunctionInstance( RegexConfiguration( args[0], selectIndex), m_errorhnd);
	}
	CATCH_ERROR_MAP_ARG1_RETURN( _TXT("error creating \"%s\" tokenizer instance: %s"), TOKENIZER_NAME, *m_errorhnd, 0);
}

const char* RegexTokenizerFunction::getDescription() const
{
	try
	{
		return _TXT("Tokenizer selecting tokens from source that are matching a regular expression.");
	}
	CATCH_ERROR_MAP_ARG1_RETURN( _TXT("error getting \"%s\" tokenizer description: %s"), TOKENIZER_NAME, *m_errorhnd, 0);
}


