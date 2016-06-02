/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "normalizerRegex.hpp"
#include "strus/errorBufferInterface.hpp"
#include "strus/normalizerFunctionContextInterface.hpp"
#include "strus/normalizerFunctionInstanceInterface.hpp"
#include "private/utils.hpp"
#include "private/errorUtils.hpp"
#include "private/internationalization.hpp"
#include <cstring>
#include <iterator>
#include <limits>
#include <boost/regex.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>

using namespace strus;

class RegexOutputFormatter
{
public:
	RegexOutputFormatter()
		:m_items(),m_strings(){}

	RegexOutputFormatter( const RegexOutputFormatter& o)
		:m_items(o.m_items),m_strings(o.m_strings){}

	explicit RegexOutputFormatter( const std::string& fmt)
	{
		char const* fi = fmt.c_str();
		const char* pre = fi;
		while (*fi)
		{
			if (*fi == '$')
			{
				if (fi > pre)
				{
					int stridx = m_strings.size()+1;
					m_strings.push_back( '\0');
					m_strings.append( pre, fi - pre);
					if (stridx > std::numeric_limits<int>::max()) throw std::runtime_error( "output formatter string size out of range");
					m_items.push_back( stridx);
				}
				++fi;
				int idx = 0;
				while (*fi >= '0' && *fi <= '9')
				{
					idx = idx * 10 + (*fi - '0');
					if (idx > std::numeric_limits<short>::max()) throw std::runtime_error( "output formatter element reference index out of range");
					++fi;
				}
				m_items.push_back( -idx);
				pre = fi;
			}
		}
		if (fi > pre)
		{
			int stridx = m_strings.size()+1;
			m_strings.push_back( '\0');
			m_strings.append( pre, fi - pre);
			if (stridx > std::numeric_limits<int>::max()) throw std::runtime_error( "output formatter string size out of range");
			m_items.push_back( stridx);
		}
	}

	std::string print( const boost::match_results<std::string::const_iterator>& match) const
	{
		std::string rt;
		std::vector<int>::const_iterator ti = m_items.begin(), te = m_items.end();
		for (; ti != te; ++ti)
		{
			if (*ti <= 0)
			{
				//... output variable
				rt.append( match.str( -*ti));
			}
			else
			{
				rt.append( m_strings.c_str() + *ti);
			}
		}
		return rt;
	}

private:
	std::vector<int> m_items;
	std::string m_strings;
};

struct RegexConfiguration
{
	boost::regex expression;
	RegexOutputFormatter formatter;
	boost::function<std::string (boost::match_results<std::string::const_iterator>)> output;

	RegexConfiguration( const std::string& expressionstr, const std::string& fmtstr)
		:expression(expressionstr)
		,formatter(fmtstr)
	{
		output = boost::bind(&RegexOutputFormatter::print, &formatter, _1);
	}
	RegexConfiguration( const RegexConfiguration& o)
		:expression(o.expression){}
};

class RegexNormalizerFunctionContext
	:public NormalizerFunctionContextInterface
{
public:
	RegexNormalizerFunctionContext( const RegexConfiguration& config_, ErrorBufferInterface* errorhnd_)
		:m_errorhnd(errorhnd_),m_config(config_){}

	virtual ~RegexNormalizerFunctionContext(){}

	virtual std::string normalize( const char* src, std::size_t srcsize)
	{
		try
		{
			return boost::regex_replace(
				std::string( src, srcsize), m_config.expression, m_config.output,
				boost::match_posix);
		}
		CATCH_ERROR_MAP_RETURN( _TXT("error executing \"regex\" normalizer function: %s"), *m_errorhnd, std::string());
	}

private:
	ErrorBufferInterface* m_errorhnd;
	RegexConfiguration m_config; 
};


class RegexNormalizerFunctionInstance
	:public NormalizerFunctionInstanceInterface
{
public:
	RegexNormalizerFunctionInstance( const RegexConfiguration& config_, ErrorBufferInterface* errorhnd_)
		:m_errorhnd(errorhnd_),m_config(config_){}

	virtual ~RegexNormalizerFunctionInstance(){}

	virtual NormalizerFunctionContextInterface* createFunctionContext() const
	{
		try
		{
			return new RegexNormalizerFunctionContext( m_config, m_errorhnd);
		}
		CATCH_ERROR_MAP_RETURN( _TXT("error creating \"regex\" normalizer function context: %s"), *m_errorhnd, 0);
	}

private:
	ErrorBufferInterface* m_errorhnd;
	RegexConfiguration m_config; 
};


NormalizerFunctionInstanceInterface* RegexNormalizerFunction::createInstance(
		const std::vector<std::string>& args,
		const TextProcessorInterface* tp) const
{
	try
	{
		if (args.size() > 2)
		{
			throw strus::runtime_error(_TXT("too many arguments for \"regex\" normalizer (two arguments, 1st the regular expression to find tokens and 2nd the format string for the output)"));
		}
		else if (args.size() < 2)
		{
			throw strus::runtime_error(_TXT("expected two arguments for \"regex\" normalizer: 1st the regular expression to find tokens and 2nd the format string for the output)"));
		}
		return new RegexNormalizerFunctionInstance( RegexConfiguration( args[0], args[1]), m_errorhnd);
	}
	CATCH_ERROR_MAP_RETURN( _TXT("error creating \"regex\" normalizer instance: %s"), *m_errorhnd, 0);
}

const char* RegexNormalizerFunction::getDescription() const
{
	try
	{
		return _TXT( "Normalizer that does a regular expression match with the first argument and a replace with the format string defined in the second argument.");
	}
	CATCH_ERROR_MAP_RETURN( _TXT("error getting \"regex\" normalizer description: %s"), *m_errorhnd, 0);
}


