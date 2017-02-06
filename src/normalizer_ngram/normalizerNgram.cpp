/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "normalizerNgram.hpp"
#include "strus/errorBufferInterface.hpp"
#include "textwolf/charset_utf8.hpp"
#include "textwolf/cstringiterator.hpp"
#include "private/utils.hpp"
#include "private/errorUtils.hpp"
#include "private/internationalization.hpp"
#include <cstring>

using namespace strus;

#define NORMALIZER_NAME "ngram"

struct NgramConfiguration
{
	unsigned int width;
	bool withEnd;
	bool withStart;
	bool roundRobin;

	NgramConfiguration()
		:width(3),withEnd(false),withStart(false),roundRobin(false){}
	NgramConfiguration( unsigned int width_, bool withEnd_, bool withStart_, bool roundRobin_)
		:width(width_),withEnd(withEnd_),withStart(withStart_),roundRobin(roundRobin_){}
	NgramConfiguration( const NgramConfiguration& o)
		:width(o.width),withEnd(o.withEnd),withStart(o.withStart),roundRobin(o.roundRobin){}
};


class NgramNormalizerInstance
	:public NormalizerFunctionInstanceInterface
{
public:
	NgramNormalizerInstance( const NgramConfiguration& config_, ErrorBufferInterface* errorhnd)
		:m_config(config_),m_errorhnd(errorhnd){}

	virtual std::string normalize(
			const char* src,
			std::size_t srcsize) const
	{
		try
		{
			std::string tok;
			if (m_config.withStart)
			{
				tok.push_back( '_');
			}
			tok.append( src, srcsize);
			if (m_config.withEnd)
			{
				tok.push_back( '_');
			}
			if (m_config.roundRobin)
			{
				std::size_t ii=0;
				for (;ii+1<m_config.width; ++ii)
				{
					tok.push_back( tok[ii]);
				}
			}
			std::string rt;
			std::size_t ti = 0, te = tok.size();
			if (te < m_config.width)
			{
				rt.push_back('\0');
				rt.append( tok);
			}
			else
			{
				for (; ti+m_config.width <= te; ++ti)
				{
					rt.push_back('\0');
					rt.append( tok.c_str()+ti, m_config.width);
				}
			}
			return rt;
		}
		CATCH_ERROR_MAP_RETURN( _TXT("error in normalize: %s"), *m_errorhnd, std::string());
	}

private:
	NgramConfiguration m_config;
	ErrorBufferInterface* m_errorhnd;
};

NormalizerFunctionInstanceInterface* NgramNormalizerFunction::createInstance( const std::vector<std::string>& args, const TextProcessorInterface*) const
{
	NgramConfiguration config;
	std::vector<std::string>::const_iterator ai = args.begin(), ae = args.end();
	for (; ai != ae; ++ai)
	{
		if ((*ai)[0] >= '0' && ((*ai)[0] <= '9'))
		{
			config.width = utils::toint( *ai);
		}
		else if (utils::caseInsensitiveEquals( *ai, "RoundRobin"))
		{
			config.roundRobin = true;
		}
		else if (utils::caseInsensitiveEquals( *ai, "WithEnd"))
		{
			config.withEnd = true;
		}
		else if (utils::caseInsensitiveEquals( *ai, "WithStart"))
		{
			config.withStart = true;
		}
		else
		{
			throw strus::runtime_error(_TXT("unknown configuration option for '%s' normalizer: '%s'"), NORMALIZER_NAME, ai->c_str());
		}
	}
	try
	{
		return new NgramNormalizerInstance( config, m_errorhnd);
	}
	CATCH_ERROR_MAP_RETURN( _TXT("error in create normalizer instance: %s"), *m_errorhnd, 0);
}

