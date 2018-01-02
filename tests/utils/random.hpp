/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef _STRUS_ANALYZER_TEST_RANDOM_HPP_INCLUDED
#define _STRUS_ANALYZER_TEST_RANDOM_HPP_INCLUDED
#include "strus/base/stdint.h"
#include <cstdarg>
#include <ctime>

namespace strus {
namespace test {

/// \brief Pseudo random generator for tests
class Random
{
public:
	enum {KnuthIntegerHashFactor=2654435761U};

	Random()
	{
		time_t nowtime;
		struct tm* now;

		::time( &nowtime);
		now = ::localtime( &nowtime);

		m_value = uint32_hash( ((now->tm_year+1)
					* (now->tm_mon+100)
					* (now->tm_mday+1)));
	}

	unsigned int get( unsigned int min_, unsigned int max_)
	{
		if (min_ >= max_)
		{
			throw std::runtime_error("illegal range passed to pseudo random number generator");
		}
		m_value = uint32_hash( m_value + 1);
		unsigned int iv = max_ - min_;
		if (iv)
		{
			return (m_value % iv) + min_;
		}
		else
		{
			return min_;
		}
	}

	unsigned int get( unsigned int min_, unsigned int max_, unsigned int psize, ...)
	{
		va_list ap;
		unsigned int pidx = get( 0, psize+1);
		if (pidx == psize)
		{
			return get( min_, max_);
		}
		else
		{
			unsigned int rt = min_;
			va_start( ap, psize);
			for (unsigned int ii = 0; ii <= pidx; ii++)
			{
				rt = va_arg( ap, unsigned int);
			}
			va_end(ap);
			return rt;
		}
	}

private:
	static uint32_t uint32_hash( uint32_t a)
	{
		a += ~(a << 15);
		a ^=  (a >> 10);
		a +=  (a << 3);
		a ^=  (a >> 6);
		a += ~(a << 11);
		a ^=  (a >> 16);
		return a;
	}

private:
	unsigned int m_value;
};

}}//namespace
#endif


