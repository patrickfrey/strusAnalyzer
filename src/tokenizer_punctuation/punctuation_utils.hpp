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
#ifndef _STRUS_TOKENIZER_PUNCTUATION_UTILS_HPP_INCLUDED
#define _STRUS_TOKENIZER_PUNCTUATION_UTILS_HPP_INCLUDED
#include "textwolf/textscanner.hpp"
#include "textwolf/cstringiterator.hpp"
#include "textwolf/charset.hpp"

namespace strus
{

class CharTable
{
public:
	CharTable( const char* op)
	{
		std::size_t ii;
		for (ii=0; ii<sizeof(m_ar); ++ii) m_ar[ii] = false;
		for (ii=0; op[ii]; ++ii)
		{
			m_ar[(unsigned char)(op[ii])] = true;
		}
	}

	bool operator[]( char ch) const		{return m_ar[ (unsigned char)ch];}
private:
	bool m_ar[256];
};

static const CharTable consonant_char("bcdfghjklmnpqrstvwxzBCDFGHJKLMNPQRSTVWXZ");

static inline bool isSpace( textwolf::UChar ch)
{
	return (ch <= 32);
}
static inline bool isConsonant( textwolf::UChar ch)
{
	return (ch <= 127 && consonant_char[(unsigned char)ch]);
}
static inline bool isDigit( textwolf::UChar ch)
{
	return (ch >= '0' && ch <= '9');
}
static inline bool isUmlaut( textwolf::UChar ch)
{
	static const unsigned char ar[] = {0xc4,0xe4,0xd6,0xf6,0xdc,0xfc,0};
	return (0!=std::strchr((const char*) ar, (char)ch));
}
static inline bool isAlpha( textwolf::UChar ch)
{
	return ((ch|32) >= 'a' && (ch|32) <= 'z') || isUmlaut( ch);
}
static inline bool isUppercase( textwolf::UChar ch)
{
	return (ch >= 'A' && ch <= 'Z') || ch == 0xc4 || ch == 0xd6 || ch == 0xdc;
}

class CharWindow
{
public:
	enum {NofPrevChar=16};
	typedef textwolf::TextScanner<
			textwolf::CStringIterator,
			textwolf::charset::UTF8>
		Scanner;

	CharWindow( const char* src, std::size_t srcsize, const CharTable* punctuation_char_)
		:m_itr( textwolf::CStringIterator( src, srcsize))
		,m_idx(0)
		,m_wordlen(0)
		,m_pos(0)
		,m_punctuation_char(punctuation_char_)
	{
		std::memset( m_prev_ch, 0, sizeof(m_prev_ch));
		m_prev_ch[ 0] = *m_itr;
		if (!isSpace( m_prev_ch[ 0]) && !isPunctuation( m_prev_ch[ 0]))
		{
			++m_wordlen;
		}
	}

	inline bool isPunctuation( textwolf::UChar ch)
	{
		return (ch <= 127 && (*m_punctuation_char)[(unsigned char)ch]);
	}

	void skip()
	{
		m_pos = m_itr.getPosition();
		++m_itr;
		++m_idx;
		textwolf::UChar ch = *m_itr;
		m_prev_ch[ m_idx & ((unsigned int)NofPrevChar -1)] = ch;
		if (!isSpace( ch) && !isPunctuation( ch))
		{
			++m_wordlen;
		}
		else
		{
			m_wordlen = 0;
		}
	}

	textwolf::UChar chr( unsigned int idx) const
	{
		return m_prev_ch[ (m_idx - idx) & ((unsigned int)NofPrevChar -1)];
	}

	unsigned int pos() const
	{
		return m_pos;
	}

	unsigned int wordlen() const
	{
		return m_wordlen;
	}

	std::string tostring()
	{
		std::string rt;
		unsigned int ii=0;
		for (; ii<NofPrevChar; ++ii)
		{
			textwolf::UChar ch = chr( NofPrevChar-ii-1);
			if (ch) rt.push_back( ch<256?(char)(unsigned char)ch:'_');
		}
		return rt;
	}

private:
	Scanner m_itr;
	unsigned int m_idx;
	unsigned int m_wordlen;
	unsigned int m_pos;
	textwolf::UChar m_prev_ch[ NofPrevChar];
	const CharTable* m_punctuation_char;
};

}//namespace
#endif

