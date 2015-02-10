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
#include "punctuation.hpp"
#include "textwolf/textscanner.hpp"
#include "textwolf/cstringiterator.hpp"
#include "textwolf/charset.hpp"
#include <cstring>
#include <iostream>
#include <boost/algorithm/string.hpp>

using namespace strus;
using namespace strus::tokenizer;

#undef STRUS_LOWLEVEL_DEBUG

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

static const CharTable punctuation_char(":.;,!?()-");
static const CharTable consonant_char("bcdfghjklmnpqrstvwxzBCDFGHJKLMNPQRSTVWXZ");

static inline bool isSpace( textwolf::UChar ch)
{
	return (ch <= 32);
}
static inline bool isPunctuation( textwolf::UChar ch)
{
	return (ch <= 127 && punctuation_char[(unsigned char)ch]);
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

class PunctuationTokenizer
	:public TokenizerInterface
{
public:
	PunctuationTokenizer(){}

	class CharWindow
	{
	public:
		enum {NofPrevChar=8};
		typedef textwolf::TextScanner<
				textwolf::CStringIterator,
				textwolf::charset::UTF8>
			Scanner;

		CharWindow( const char* src, std::size_t srcsize)
			:m_itr( textwolf::CStringIterator( src, srcsize))
			,m_idx(0)
			,m_wordlen(0)
			,m_pos(0)
		{
			std::memset( m_prev_ch, 0, sizeof(m_prev_ch));
			m_prev_ch[ 0] = *m_itr;
			if (!isSpace( m_prev_ch[ 0]) && !isPunctuation( m_prev_ch[ 0]))
			{
				++m_wordlen;
			}
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
	};

	virtual bool concatBeforeTokenize() const
	{
		return true;
	}

	Argument* createArgument( const std::vector<std::string>& arg) const
	{
		if (arg.size() != 1)
		{
			throw std::runtime_error("illegal number of arguments for punctuation tokenizer (language as single argument expected)");
		}
		if (boost::algorithm::iequals( arg[0], "de") || boost::algorithm::iequals( arg[0], "D"))
		{
			return 0;
		}
		throw std::runtime_error( std::string("unsupported language passed to punctuation tokenizer ('") + arg[0] + "'");
	}

	virtual std::vector<Token> tokenize( Context*, const char* src, std::size_t srcsize) const
	{
		std::vector<Token> rt;

		textwolf::UChar ch0;
		CharWindow scanner( src, srcsize);
		unsigned int wordlen=0;

		for (; 0!=(ch0=scanner.chr(0)); wordlen=scanner.wordlen(),scanner.skip())
		{
			if (ch0 == '-')
			{
				textwolf::UChar ch1 = scanner.chr(1);
				if (isAlpha( ch1)) continue;
			}
			else if (ch0 == '.')
			{
				textwolf::UChar ch1 = scanner.chr(1);
				if (isDigit( ch1))
				{
					// dot in a number belongs to the number
#ifdef STRUS_LOWLEVEL_DEBUG
					std::cout << "ABBREV " << scanner.tostring() << std::endl;
#endif
					continue;
				}
				if (isDigit( ch1) || wordlen == 1)
				{
					// single characters followed by a dot.
#ifdef STRUS_LOWLEVEL_DEBUG
					std::cout << "ABBREV " << (int)__LINE__ << ":" << scanner.tostring() << std::endl;
#endif
					continue;
				}
				if (isAlpha( ch1))
				{
					textwolf::UChar ch2 = scanner.chr(2);
					textwolf::UChar ch3 = scanner.chr(3);
					if (ch3)
					{
						// 3 subsequent consonants at the end
						// or 2 subsequent consonants in a word
						// with length <= 3
						if (isConsonant( ch1)
						&&  isConsonant( ch2) && ch2 != ch1)
						{
							if ((wordlen <= 3 
								&& (ch3 != 'i' && ch2 != 's' && ch1 != 't')
								&& (ch2 != 'h' && ch1 != 'r' && ch1 != 'n' && ch1 != 't')
								&& (ch2 != 'h' && ch1 != 'r')
								&& (ch3 != 'u' && ch2 != 'n' && ch1 != 's'))
							|| ((isConsonant( ch3) && ch3 != ch2 && wordlen <= 5)
								&& (ch3 != 'c' && ch2 != 'h' && ch1 != 't')
								&& (ch3 != 'h' && ch2 != 'r' && ch1 != 't')
							))
							{
#ifdef STRUS_LOWLEVEL_DEBUG
								std::cout << "ABBREV " << (int)__LINE__ << ":" << scanner.tostring() << std::endl;
#endif
								continue;
							}
						}
					}
					else
					{
						if (isConsonant( ch1) && isConsonant( ch2))
						{
#ifdef STRUS_LOWLEVEL_DEBUG
							std::cout << "ABBREV " << (int)__LINE__ << ":" << scanner.tostring() << std::endl;
#endif
							continue;
						}
					}
					if (ch1 == 'f' && ch2 == 'o' && ch3 == 'r')
					{
						// special case for "Prof."
#ifdef STRUS_LOWLEVEL_DEBUG
						std::cout << "ABBREV " << (int)__LINE__ << ":" << scanner.tostring() << std::endl;
#endif
						continue;
					}
					if (ch1 == 'c' && wordlen <= 3)
					{
						// c at the end of a small word
#ifdef STRUS_LOWLEVEL_DEBUG
						std::cout << "ABBREV " << (int)__LINE__ << ":" << scanner.tostring() << std::endl;
#endif
						continue;
					}
					if (ch1 == 'z' && ch2 != 't' && ch2 != 'n' && ch2 != 'r' && ch2 != 'l' && isConsonant( ch2))
					{
						// z after another consonant at the end a word
#ifdef STRUS_LOWLEVEL_DEBUG
						std::cout << "ABBREV " << (int)__LINE__ << ":" << scanner.tostring() << std::endl;
#endif
						continue;
					}
					if (ch1 == 'w' && isConsonant( ch2))
					{
						// w after another consonant at the end a word
#ifdef STRUS_LOWLEVEL_DEBUG
						std::cout << "ABBREV " << (int)__LINE__ << ":" << scanner.tostring() << std::endl;
#endif
						continue;
					}
					if (ch1 == 'v' && isConsonant( ch2))
					{
						// v after another consonant at the end a word
#ifdef STRUS_LOWLEVEL_DEBUG
						std::cout << "ABBREV " << (int)__LINE__ << ":" << scanner.tostring() << std::endl;
#endif
						continue;
					}
					if (ch1 == 'h' && ch2 != 'c' && isConsonant( ch2))
					{
						// h after a consonant other than 'c' at the end a word
#ifdef STRUS_LOWLEVEL_DEBUG
						std::cout << "ABBREV " << (int)__LINE__ << ":" << scanner.tostring() << std::endl;
#endif
						continue;
					}
					if (ch1 == 'k' && ch2 != 'c' && ch2 != 'l' && ch2 != 'n' && ch2 != 'r' && isConsonant( ch2))
					{
						// k after a consonant other than 'c','l','n' or 'r' at the end a word
#ifdef STRUS_LOWLEVEL_DEBUG
						std::cout << "ABBREV " << (int)__LINE__ << ":" << scanner.tostring() << std::endl;
#endif
						continue;
					}
				}
#ifdef STRUS_LOWLEVEL_DEBUG
				std::cout << "PUNKT " << (int)__LINE__ << ":" << scanner.tostring() << std::endl;
#endif
				rt.push_back( Token( scanner.pos(), 1));
			}
			else if (isPunctuation(ch0))
			{
				rt.push_back( Token( scanner.pos(), 1));
			}
		}
		return rt;
	}
};

const TokenizerInterface* strus::punctuationTokenizer()
{
	static const PunctuationTokenizer tokenizer;
	return &tokenizer;
}


