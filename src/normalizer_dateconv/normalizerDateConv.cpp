/*
---------------------------------------------------------------------
    The C++ library strus implements basic operations to build
    a search engine for structured search on unstructured data.

    Copyright (C) 2013,2014 Patrick Frey

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

--------------------------------------------------------------------

	The latest version of strus can be found at 'http://github.com/patrickfrey/strus'
	For documentation see 'http://patrickfrey.github.com/strus'

--------------------------------------------------------------------
*/
#include "normalizerDateConv.hpp"
#include "strus/analyzerErrorBufferInterface.hpp"
#include "private/utils.hpp"
#include "private/errorUtils.hpp"
#include "private/internationalization.hpp"
#include <cstring>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <map>
#include <vector>
#include <boost/algorithm/string.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time.hpp>
#include <boost/cstdint.hpp>

using namespace strus;

struct DateNumGranularity
{
	enum Type
	{
		Microsecond,
		Millisecond,
		Second,
		Minute,
		Hour,
		Day
	};
	DateNumGranularity( Type type_, const boost::posix_time::ptime& start_, unsigned int factor_)
		:m_type(type_),m_start(start_),m_factor(factor_){}
	DateNumGranularity( const DateNumGranularity& o)
		:m_type(o.m_type),m_start(o.m_start),m_factor(o.m_factor){}

	Type m_type;
	boost::posix_time::ptime m_start;
	unsigned m_factor;

	boost::int64_t getValue( const boost::posix_time::ptime& timstmp)
	{
		typedef boost::posix_time::time_duration time_duration;
		time_duration diff = timstmp - m_start;
		switch (m_type)
		{
			case Microsecond:
				return diff.ticks() / ((time_duration::ticks_per_second() / 1000000) * m_factor);
			case Millisecond:
				return diff.ticks() / ((time_duration::ticks_per_second() / 1000) * m_factor);
			case Second:
				return diff.ticks() / ((time_duration::ticks_per_second()) * m_factor);
			case Minute:
				return diff.ticks() / ((time_duration::ticks_per_second()) * (60 * m_factor));
			case Hour:
				return diff.ticks() / ((time_duration::ticks_per_second()) * (3600 * m_factor));
			case Day:
				return diff.ticks() / ((time_duration::ticks_per_second()) * (24 * 3600 * m_factor));
		}
		throw strus::runtime_error(_TXT("unexpected error in time calculation"));
	}
};


class Date2IntNormalizerFunctionContext
	:public NormalizerFunctionContextInterface
{
public:
	Date2IntNormalizerFunctionContext( const DateNumGranularity& granularity_, const std::vector<std::locale>& lcar_, AnalyzerErrorBufferInterface* errorhnd)
		:m_granularity(granularity_),m_lcar(lcar_),m_errorhnd(errorhnd)
	{}

	virtual std::string normalize(
			const char* src,
			std::size_t srcsize)
	{
		try
		{
			std::string inputstr( src, srcsize);
			boost::posix_time::ptime pt;
			std::vector<std::locale>::const_iterator li = m_lcar.begin(), le = m_lcar.end();
			for (; li != le; ++li)
			{
				std::istringstream is( inputstr);
				is.imbue( *li);
				is >> pt;
				if(pt != boost::posix_time::ptime()) break;
			}
			std::ostringstream out;
			out << m_granularity.getValue( pt);
			return out.str();
		}
		CATCH_ERROR_MAP_RETURN( _TXT("error in 'date2int' normalizer: %s"), *m_errorhnd, std::string());
	}

private:
	DateNumGranularity m_granularity;
	std::vector<std::locale> m_lcar;
	AnalyzerErrorBufferInterface* m_errorhnd;
};

class Date2IntNormalizerFunctionInstance
	:public NormalizerFunctionInstanceInterface
{
public:
	Date2IntNormalizerFunctionInstance( const DateNumGranularity& granularity_, const std::vector<std::locale>& lcar_, AnalyzerErrorBufferInterface* errorhnd)
		:m_granularity(granularity_),m_lcar(lcar_),m_errorhnd(errorhnd){}

	virtual NormalizerFunctionContextInterface* createFunctionContext() const
	{
		try
		{
			return new Date2IntNormalizerFunctionContext( m_granularity, m_lcar, m_errorhnd);
		}
		CATCH_ERROR_MAP_RETURN( _TXT("error in 'date2int' normalizer: %s"), *m_errorhnd, 0);
	}

private:
	DateNumGranularity m_granularity;
	std::vector<std::locale> m_lcar;
	AnalyzerErrorBufferInterface* m_errorhnd;
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
	if (gi[0] == 'u' && gi[1] == 's')
	{
		if (isAlphaNum(gi[2])) throw strus::runtime_error( _TXT("error in result definition: unknown time unit identifier"));
		gi+=2;
		return DateNumGranularity::Microsecond;
	}
	else if (gi[0] == 'm' && gi[1] == 's')
	{
		if (isAlphaNum(gi[2])) throw strus::runtime_error( _TXT("error in result definition: unknown time unit identifier"));
		gi+=2;
		return DateNumGranularity::Millisecond;
	}
	else if (*gi == 's')
	{
		if (isAlphaNum(gi[1])) throw strus::runtime_error( _TXT("error in result definition: unknown time unit identifier"));
		gi++;
		return DateNumGranularity::Second;
	}
	else if (*gi == 'm')
	{
		if (isAlphaNum(gi[1])) throw strus::runtime_error( _TXT("error in result definition: unknown time unit identifier"));
		gi++;
		return DateNumGranularity::Minute;
	}
	else if (*gi == 'h')
	{
		if (isAlphaNum(gi[1])) throw strus::runtime_error( _TXT("error in result definition: unknown time unit identifier"));
		gi++;
		return DateNumGranularity::Hour;
	}
	else if (*gi == 'd')
	{
		if (isAlphaNum(gi[1])) throw strus::runtime_error( _TXT("error in result definition: unknown time unit identifier"));
		gi++;
		return DateNumGranularity::Day;
	}
	else if (*gi == 'y')
	{
		if (isAlphaNum(gi[1])) throw strus::runtime_error( _TXT("error in result definition: unknown time unit identifier"));
		gi++;
		return DateNumGranularity::Day;
	}
	throw strus::runtime_error( _TXT("error in result definition: unknown time unit identifier"));
}

static unsigned int parseNumber( char const*& gi)
{
	unsigned int rt = 0;
	gi = skipSpaces( gi+1);
	for (; *gi && *gi >= '0' && *gi <= '9'; ++gi)
	{
		unsigned int fo = rt;
		rt = rt * 10 + (*gi - '0');
		if (fo < rt) throw strus::runtime_error( _TXT("error in result definition: number out of range"));
	}
	if (!rt)
	{
		throw strus::runtime_error( _TXT("error in result definition: number expected"));
	}
	return rt;
}

DateNumGranularity parseGranularity( char const* gi)
{
	DateNumGranularity::Type type = parseGranularityType( gi);
	boost::posix_time::ptime start;
	unsigned int factor = 1;

	gi = skipSpaces( gi);
	if (*gi == '/')
	{
		++gi;
		factor = parseNumber( gi);
	}
	gi = skipSpaces( gi);
	if (*gi)
	{
		std::string fromDate( gi);
		if (std::strchr( gi, ' ') == 0)
		{
			fromDate.append( " 00:00:00");
		}
		try
		{
			start = boost::posix_time::time_from_string( fromDate);
		}
		catch (...)
		{
			throw strus::runtime_error( _TXT("error in result definition: illegal start time"));
		}
	}
	else
	{
		start = boost::posix_time::ptime( boost::gregorian::date( 1970, 1, 1));
	}
	return DateNumGranularity( type, start, factor);
}


NormalizerFunctionInstanceInterface* Date2IntNormalizerFunction::createInstance( const std::vector<std::string>& args, const TextProcessorInterface*) const
{
	try
	{
		if (args.size() < 2)
		{
			m_errorhnd->report( _TXT("too few arguments (%u)"), (unsigned int)args.size());
			return 0;
		}
		std::vector<std::string>::const_iterator ai = args.begin(), ae = args.end();
	
		DateNumGranularity granularity( parseGranularity( ai->c_str()));
	
		std::vector<std::locale> lcar;
		for (++ai; ai != ae; ++ai)
		{
			lcar.push_back(
				std::locale(std::locale::classic(),
				new boost::posix_time::time_input_facet( ai->c_str())));
		}
		return new Date2IntNormalizerFunctionInstance( granularity, lcar, m_errorhnd);
	}
	CATCH_ERROR_MAP_RETURN( _TXT("error in 'date2int' normalizer: %s"), *m_errorhnd, 0);
}


