/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "normalizerRegex.hpp"
#include "strus/errorBufferInterface.hpp"
#include "strus/normalizerFunctionInstanceInterface.hpp"
#include "strus/normalizerFunctionInterface.hpp"
#include "strus/base/regex.hpp"
#include "private/errorUtils.hpp"
#include "private/internationalization.hpp"
#include "strus/analyzer/functionView.hpp"
#include <cstring>
#include <iterator>
#include <limits>

using namespace strus;

#define MODULE_NAME "regex"

class RegexSubstNormalizerFunctionInstance
	:public NormalizerFunctionInstanceInterface
{
public:
	RegexSubstNormalizerFunctionInstance( const std::string& expr_, const std::string& result_, ErrorBufferInterface* errorhnd_)
		:m_errorhnd(errorhnd_),m_expr(expr_),m_result(result_),m_subst( expr_, result_, errorhnd_)
	{
		if (m_errorhnd->hasError())
		{
			throw std::runtime_error( m_errorhnd->fetchError());
		}
	}

	virtual ~RegexSubstNormalizerFunctionInstance(){}

	virtual std::string normalize( const char* src, std::size_t srcsize) const
	{
		std::string rt;
		if (!m_subst.exec( rt, src, srcsize))
		{
			m_errorhnd->report( ErrorCodeInvalidRegex, _TXT("failed to match regular expression in normalizer function '%s'"), "regex");
			return std::string();
		}
		else
		{
			return rt;
		}
	}

	virtual analyzer::FunctionView view() const
	{
		try
		{
			return analyzer::FunctionView( MODULE_NAME)
				( "expression", m_expr)
				( "result", m_result)
			;
		}
		CATCH_ERROR_MAP_RETURN( _TXT("error in introspection: %s"), *m_errorhnd, analyzer::FunctionView());
	}

private:
	ErrorBufferInterface* m_errorhnd;
	std::string m_expr;
	std::string m_result;
	strus::RegexSubst m_subst; 
};


class RegexSelectNormalizerFunctionInstance
	:public NormalizerFunctionInstanceInterface
{
public:
	RegexSelectNormalizerFunctionInstance( const std::string& expr_, ErrorBufferInterface* errorhnd_)
		:m_errorhnd(errorhnd_),m_expr(expr_),m_search( expr_, 0, errorhnd_),m_startMatchOnly(!expr_.empty() && expr_[0] == '^')
	{
		if (m_errorhnd->hasError())
		{
			throw std::runtime_error( m_errorhnd->fetchError());
		}
	}

	virtual ~RegexSelectNormalizerFunctionInstance(){}

	virtual std::string normalize( const char* src, std::size_t srcsize) const
	{
		RegexSearch::Match match = m_search.find_in( src, srcsize);
		if (!match.valid())
		{
			return std::string("\0",1);
		}
		else if (m_startMatchOnly)
		{
			return std::string( src + match.pos, match.len);
		}
		else
		{
			std::size_t nextpos = match.pos + (match.len ? match.len : 1);
			if (srcsize >= nextpos)
			{
				RegexSearch::Match next_match = m_search.find_in( src + nextpos, srcsize - nextpos);
				if (next_match.valid())
				{
					std::string rt;
					rt.push_back('\0');
					rt.append( src + match.pos, match.len);
					rt.push_back('\0');
					rt.append( src + nextpos + next_match.pos, next_match.len);
	
					nextpos += next_match.pos + (next_match.len ? next_match.len : 1);
					if (srcsize >= nextpos)
					{
						next_match = m_search.find_in( src + nextpos, srcsize - nextpos);
						while (next_match.valid())
						{
							rt.push_back('\0');
							rt.append( src + nextpos + next_match.pos, next_match.len);
							nextpos += next_match.pos + (next_match.len ? next_match.len : 1);
							if (srcsize < nextpos) break;
							next_match = m_search.find_in( src + nextpos, srcsize - nextpos);
						}
					}
					return rt;
				}
				else
				{
					return std::string( src + match.pos, match.len);
				}
			}
			else
			{
				return std::string( src + match.pos, match.len);
			}
		}
	}

	virtual analyzer::FunctionView view() const
	{
		try
		{
			return analyzer::FunctionView( MODULE_NAME)
				( "expression", m_expr)
			;
		}
		CATCH_ERROR_MAP_RETURN( _TXT("error in introspection: %s"), *m_errorhnd, analyzer::FunctionView());
	}

private:
	ErrorBufferInterface* m_errorhnd;
	std::string m_expr;
	strus::RegexSearch m_search; 
	bool m_startMatchOnly;
};


NormalizerFunctionInstanceInterface* RegexNormalizerFunction::createInstance(
		const std::vector<std::string>& args,
		const TextProcessorInterface* tp) const
{
	try
	{
		if (args.size() > 2)
		{
			throw strus::runtime_error( _TXT("too many arguments for \"%s\" normalizer (two arguments, 1st the regular expression to find tokens and 2nd the format string for the output)"), MODULE_NAME);
		}
		else if (args.size() < 1)
		{
			throw strus::runtime_error( _TXT("expected two arguments for \"%s\" normalizer: 1st the regular expression to find tokens and 2nd the format string for the output)"), MODULE_NAME);
		}
		else if (args.size() == 1)
		{
			return new RegexSelectNormalizerFunctionInstance( args[0], m_errorhnd);
		}
		else
		{
			return new RegexSubstNormalizerFunctionInstance( args[0], args[1], m_errorhnd);
		}
	}
	CATCH_ERROR_ARG1_MAP_RETURN( _TXT("error creating \"%s\" normalizer instance: %s"), MODULE_NAME, *m_errorhnd, 0);
}

const char* RegexNormalizerFunction::getDescription() const
{
	try
	{
		return _TXT( "Normalizer that does a regular expression match with the first argument and a replace with the format string defined in the second argument if one is defined or simply return the matches if no second argument is specified.");
	}
	CATCH_ERROR_ARG1_MAP_RETURN( _TXT("error getting \"%s\" normalizer description: %s"), MODULE_NAME, *m_errorhnd, 0);
}


