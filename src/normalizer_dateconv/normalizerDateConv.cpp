/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "normalizerDateConv.hpp"
#include "strus/errorBufferInterface.hpp"
#include "private/errorUtils.hpp"
#include "private/internationalization.hpp"
#include "strus/base/stdint.h"
#include <cstring>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <map>
#include <vector>
#include <ctime>

using namespace strus;

#define MODULENAME "date2int"

typedef struct tm TimeStruct;

struct DateNumGranularity
{
	enum Type
	{
		Second,
		Minute,
		Hour,
		Day
	};
	static const char* typeName( Type i)
	{
		static const char* ar[] = {"Second","Minute","Hour","Day"};
		return ar[(int)i];
	}

	DateNumGranularity( Type type_, const TimeStruct& start_, unsigned int factor_)
		:m_type(type_),m_start(start_),m_factor(factor_){}
	DateNumGranularity( const DateNumGranularity& o)
		:m_type(o.m_type),m_start(o.m_start),m_factor(o.m_factor){}

	Type m_type;
	TimeStruct m_start;
	unsigned m_factor;

	static uint64_t daynum( const TimeStruct& timstmp)
	{
		uint64_t yy = timstmp.tm_year + 1900;
		uint64_t mm = timstmp.tm_mon + 1;
		if (timstmp.tm_mon <= 1)
		{
			mm += 12;
			yy -= 1;
		}
		uint64_t dd = timstmp.tm_mday;
		if (!dd) dd=1;
		return (146097*yy)/400 + (153*mm + 8)/5 + dd;
	}
	static uint64_t hournum( const TimeStruct& timstmp)
	{
		return (daynum(timstmp) * 24 + timstmp.tm_hour);
	}
	static uint64_t minnum( const TimeStruct& timstmp)
	{
		return (hournum(timstmp) * 60 + timstmp.tm_min);
	}
	static uint64_t secnum( const TimeStruct& timstmp)
	{
		return (minnum(timstmp) * 60 + timstmp.tm_sec);
	}

	int64_t getValue( const TimeStruct& timstmp) const
	{
		int64_t aa = 0;
		int64_t bb = 0;
		switch (m_type)
		{
			case Second:
				aa = secnum( timstmp);
				bb = secnum( m_start);
				break;
			case Minute:
				aa = minnum( timstmp);
				bb = minnum( m_start);
				break;
			case Hour:
				aa = hournum( timstmp);
				bb = hournum( m_start);
				break;
			case Day:
				aa = daynum( timstmp);
				bb = daynum( m_start);
				break;
		}
		return (aa - bb) / m_factor;
	}
};

struct Date2IntNormalizerConfig
{
	DateNumGranularity granularity;
	std::vector<std::string> fmtar;

	Date2IntNormalizerConfig( const DateNumGranularity& granularity_, const std::vector<std::string>& fmtar_)
		:granularity(granularity_),fmtar(fmtar_){}
	Date2IntNormalizerConfig( const Date2IntNormalizerConfig& o)
		:granularity(o.granularity),fmtar(o.fmtar){}
};


class Date2IntNormalizerFunctionInstance
	:public NormalizerFunctionInstanceInterface
{
public:
	Date2IntNormalizerFunctionInstance( const DateNumGranularity& granularity_, const std::vector<std::string>& fmtar_, ErrorBufferInterface* errorhnd)
		:m_config(granularity_,fmtar_),m_errorhnd(errorhnd){}

	virtual std::string normalize(
			const char* src,
			std::size_t srcsize) const
	{
		try
		{
			if (srcsize == 0) return std::string();

			TimeStruct result;

			std::vector<std::string>::const_iterator ci = m_config.fmtar.begin(), ce = m_config.fmtar.end();
			for (; ci != ce; ++ci)
			{
				std::memset( &result, 0, sizeof(result));
				char const* pi = ::strptime( src, ci->c_str(), &result);
				if (pi == 0) continue;
				const char* pe = src + srcsize;
				for (; pi < pe && std::isspace(*pi); ++pi){}
				if (pi != pe) continue;
				break;
			}
			if (ci == ce)
			{
				std::string inputstr( src, srcsize);
				throw strus::runtime_error(_TXT("unknown time format: '%s'"), inputstr.c_str());
			}
			std::mktime( &result);
			int64_t rtnum = m_config.granularity.getValue( result);
			std::ostringstream out;
			out << rtnum;
			return out.str();
		}
		CATCH_ERROR_ARG1_MAP_RETURN( _TXT("error in '%s' normalizer: %s"), MODULENAME, *m_errorhnd, std::string());
	}

private:
	Date2IntNormalizerConfig m_config;
	ErrorBufferInterface* m_errorhnd;
};

static const char* skipSpaces( char const* gi)
{
	for (; *gi && *gi <= 32; ++gi){}
	return gi;
}

static bool isAlphaNum( char ch)
{
	if ((ch|32) >= 'a' && (ch|32) <= 'z') return true;
	if (ch >= '0' && ch <= '9') return true;
	if (ch == '_') return true;
	return false;
}

static DateNumGranularity::Type parseGranularityType( char const*& gi)
{
	if (*gi == 's')
	{
		if (isAlphaNum(gi[1])) throw strus::runtime_error( "%s", _TXT("error in result definition: unknown time unit identifier"));
		gi++;
		return DateNumGranularity::Second;
	}
	else if (*gi == 'm')
	{
		if (isAlphaNum(gi[1])) throw strus::runtime_error( "%s", _TXT("error in result definition: unknown time unit identifier"));
		gi++;
		return DateNumGranularity::Minute;
	}
	else if (*gi == 'h')
	{
		if (isAlphaNum(gi[1])) throw strus::runtime_error( "%s", _TXT("error in result definition: unknown time unit identifier"));
		gi++;
		return DateNumGranularity::Hour;
	}
	else if (*gi == 'd')
	{
		if (isAlphaNum(gi[1])) throw strus::runtime_error( "%s", _TXT("error in result definition: unknown time unit identifier"));
		gi++;
		return DateNumGranularity::Day;
	}
	else if (*gi == 'y')
	{
		if (isAlphaNum(gi[1])) throw strus::runtime_error( "%s", _TXT("error in result definition: unknown time unit identifier"));
		gi++;
		return DateNumGranularity::Day;
	}
	throw strus::runtime_error( "%s", _TXT("error in result definition: unknown time unit identifier"));
}

static unsigned int parseNumber( char const*& gi)
{
	unsigned int rt = 0;
	gi = skipSpaces( gi+1);
	for (; *gi && *gi >= '0' && *gi <= '9'; ++gi)
	{
		unsigned int fo = rt;
		rt = rt * 10 + (*gi - '0');
		if (fo < rt) throw strus::runtime_error( "%s", _TXT("error in result definition: number out of range"));
	}
	if (!rt)
	{
		throw strus::runtime_error( "%s", _TXT("error in result definition: number expected"));
	}
	return rt;
}

DateNumGranularity parseGranularity( char const* gi)
{
	DateNumGranularity::Type type = parseGranularityType( gi);
	TimeStruct start;
	std::memset( &start, 0, sizeof(start));
	unsigned int factor = 1;

	gi = skipSpaces( gi);
	if (*gi == '/')
	{
		++gi;
		factor = parseNumber( gi);
		if (factor == 0) throw strus::runtime_error( "%s", _TXT("illegal factor"));
	}
	gi = skipSpaces( gi);
	if (*gi)
	{
		char const* pi = ::strptime( gi, "%Y-%m-%d %H:%M:%S", &start);
		if (!pi) pi = ::strptime( gi, "%Y-%m-%d %H:%M", &start);
		if (!pi) pi = ::strptime( gi, "%Y-%m-%d %H", &start);
		if (!pi) pi = ::strptime( gi, "%Y-%m-%d", &start);
		if (!pi || *pi) throw strus::runtime_error(_TXT("illegal time format for start time: %s"), gi);
	}
	else
	{
		start.tm_year = 70;
		start.tm_mday = 1;
	}
	std::mktime( &start);
	return DateNumGranularity( type, start, factor);
}

NormalizerFunctionInstanceInterface* Date2IntNormalizerFunction::createInstance( const std::vector<std::string>& args, const TextProcessorInterface*) const
{
	try
	{
		if (args.size() == 0)
		{
			DateNumGranularity granularity( parseGranularity( "d"));
			std::vector<std::string> defaultFacets;
			defaultFacets.push_back( "%Y/%m/%d");
			defaultFacets.push_back( "%Y-%m-%d");
			defaultFacets.push_back( "%d.%m.%Y");
			return new Date2IntNormalizerFunctionInstance( granularity, defaultFacets, m_errorhnd);
		}
		else
		{
			std::vector<std::string>::const_iterator ai = args.begin(), ae = args.end();
			DateNumGranularity granularity( parseGranularity( ai->c_str()));
			std::vector<std::string> facets( ++ai, ae);
			return new Date2IntNormalizerFunctionInstance( granularity, facets, m_errorhnd);
		}
	}
	CATCH_ERROR_ARG1_MAP_RETURN( _TXT("error in '%s' normalizer: %s"), MODULENAME, *m_errorhnd, 0);
}


