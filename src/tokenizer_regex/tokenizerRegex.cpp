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
#include "strus/base/regex.hpp"
#include "private/utils.hpp"
#include "private/errorUtils.hpp"
#include "private/internationalization.hpp"
#include <cstring>

using namespace strus;

#define TOKENIZER_NAME "regex"

class RegexTokenizerFunctionInstance
	:public TokenizerFunctionInstanceInterface
{
public:
	RegexTokenizerFunctionInstance( const std::string& expression, int index, ErrorBufferInterface* errorhnd_)
		:m_errorhnd(errorhnd_),m_search( expression, index, errorhnd_)
	{
		if (m_errorhnd->hasError())
		{
			throw std::runtime_error( m_errorhnd->fetchError());
		}
	}

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
			char const* si = src;
			char const* se = src + srcsize;
			RegexSearch::Match mt = m_search.find( si, se);
			for (; mt.pos >= 0; si += (mt.len ? (mt.pos + mt.len) : (mt.pos + 1)), mt = m_search.find( si, se))
			{
				std::size_t abspos = mt.pos + (si - src);
				rt.push_back( analyzer::Token( abspos/*ord*/, 0/*seg*/, abspos, mt.len));
				if (abspos + mt.len >= srcsize) break;
			}
			return rt;
		}
		CATCH_ERROR_MAP_ARG1_RETURN( _TXT("error executing \"%s\" tokenizer function: %s"), TOKENIZER_NAME, *m_errorhnd, std::vector<analyzer::Token>());
	}

private:
	ErrorBufferInterface* m_errorhnd;
	RegexSearch m_search;
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
		return new RegexTokenizerFunctionInstance( args[0], selectIndex, m_errorhnd);
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


