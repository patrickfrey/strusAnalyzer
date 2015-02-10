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
#include "strus/lib/tokenizer_punctuation.hpp"
#include "strus/tokenizer/token.hpp"
#include "dll_tags.hpp"
#include <vector>
#include <string>

using namespace strus;
using namespace strus::tokenizer;

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


class WordSeparationTokenizer
	:public TokenizerInterface
{
public:
	WordSeparationTokenizer(){}

	virtual std::vector<Token> tokenize( Context*, const char* src, std::size_t srcsize) const
	{
		static const CharTable delimiter(":.;,!?%/()+-'\"`=");
		std::vector<Token> rt;
		std::size_t lastPos=0;
		std::size_t ii=0;
		for (;ii<srcsize; ++ii)
		{
			if ((unsigned char)src[ii] <= 32 || delimiter[ src[ii]])
			{
				if (ii > lastPos)
				{
					rt.push_back( Token( lastPos, ii-lastPos));
				}
				lastPos = ii+1;
			}
		}
		if (ii > lastPos)
		{
			rt.push_back( Token( lastPos, ii-lastPos));
		}
		return rt;
	}
};


class WhiteSpaceTokenizer
	:public TokenizerInterface
{
public:
	WhiteSpaceTokenizer(){}

	virtual std::vector<Token> tokenize( Context*, const char* src, std::size_t srcsize) const
	{
		std::vector<Token> rt;
		std::size_t lastPos=0;
		std::size_t ii=0;
		for (;ii<srcsize; ++ii)
		{
			if ((unsigned char)src[ii] <= 32)
			{
				if (ii > lastPos)
				{
					rt.push_back( Token( lastPos, ii-lastPos));
				}
				lastPos = ii+1;
			}
		}
		if (ii > lastPos)
		{
			rt.push_back( Token( lastPos, ii-lastPos));
		}
		return rt;
	}
};

static const WordSeparationTokenizer wordSeparationTokenizer;
static const WhiteSpaceTokenizer whiteSpaceTokenizer;



DLL_PUBLIC const TokenizerInterface* getTokenizer_word()
{
	return &wordSeparationTokenizer;
}

DLL_PUBLIC const TokenizerInterface* getTokenizer_whitespace()
{
	return &wordSeparationTokenizer;
}


