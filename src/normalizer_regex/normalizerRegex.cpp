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
#include "strus/base/introspection.hpp"
#include "private/errorUtils.hpp"
#include "private/internationalization.hpp"
#include "strus/analyzer/functionView.hpp"
#include <cstring>
#include <iterator>
#include <limits>

using namespace strus;

#define MODULE_NAME "regex"

class RegexNormalizerFunctionInstance
	:public NormalizerFunctionInstanceInterface
{
public:
	RegexNormalizerFunctionInstance( const std::string& expr_, const std::string& result_, ErrorBufferInterface* errorhnd_)
		:m_errorhnd(errorhnd_),m_expr(expr_),m_result(result_),m_subst( expr_, result_, errorhnd_)
	{
		if (m_errorhnd->hasError())
		{
			throw std::runtime_error( m_errorhnd->fetchError());
		}
	}

	virtual ~RegexNormalizerFunctionInstance(){}

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

	virtual IntrospectionInterface* createIntrospection() const
	{
		class Description :public StructTypeIntrospectionDescription<RegexNormalizerFunctionInstance>{
		public:
			Description()
			{
				(*this)
				[ "regex"]
				( "expr", &RegexNormalizerFunctionInstance::m_expr, AtomicTypeIntrospection<std::string>::constructor)
				( "result", &RegexNormalizerFunctionInstance::m_result, AtomicTypeIntrospection<std::string>::constructor)
				;
			}
		};
		static const Description descr;
		try
		{
			return new StructTypeIntrospection<RegexNormalizerFunctionInstance>( this, &descr, m_errorhnd);
		}
		CATCH_ERROR_MAP_RETURN( _TXT("error in introspection: %s"), *m_errorhnd, NULL);
	}

private:
	ErrorBufferInterface* m_errorhnd;
	std::string m_expr;
	std::string m_result;
	strus::RegexSubst m_subst; 
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
		else if (args.size() < 2)
		{
			throw strus::runtime_error( _TXT("expected two arguments for \"%s\" normalizer: 1st the regular expression to find tokens and 2nd the format string for the output)"), MODULE_NAME);
		}
		return new RegexNormalizerFunctionInstance( args[0], args[1], m_errorhnd);
	}
	CATCH_ERROR_ARG1_MAP_RETURN( _TXT("error creating \"%s\" normalizer instance: %s"), MODULE_NAME, *m_errorhnd, 0);
}

const char* RegexNormalizerFunction::getDescription() const
{
	try
	{
		return _TXT( "Normalizer that does a regular expression match with the first argument and a replace with the format string defined in the second argument.");
	}
	CATCH_ERROR_ARG1_MAP_RETURN( _TXT("error getting \"%s\" normalizer description: %s"), MODULE_NAME, *m_errorhnd, 0);
}


