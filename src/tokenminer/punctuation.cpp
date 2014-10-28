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
#include "chartable.hpp"
#include "textwolf/textscanner.hpp"
#include "textwolf/cstringiterator.hpp"
#include "textwolf/charset.hpp"
#include <cstring>

using namespace strus;

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

class PunctuationTokenizer_de
	:public TokenizerInterface
{
public:
	PunctuationTokenizer_de(){}

	class CharWindow
	{
	public:
		enum {NofPrevChar=4};
		typedef textwolf::TextScanner<
				textwolf::CStringIterator,
				textwolf::charset::UTF8>
			Scanner;

		CharWindow( const char* src, std::size_t srcsize)
			:m_itr( textwolf::CStringIterator( src, srcsize))
			,m_idx(0)
			,m_wordlen(0)
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
			return m_itr.getPosition();
		}

		unsigned int wordlen() const
		{
			return m_wordlen;
		}

	private:
		Scanner m_itr;
		unsigned int m_idx;
		unsigned int m_wordlen;
		textwolf::UChar m_prev_ch[ NofPrevChar];
	};

	virtual std::vector<Position> tokenize( const char* src, std::size_t srcsize) const
	{
		std::vector<Position> rt;

		textwolf::UChar ch0;
		CharWindow scanner( src, srcsize);
		unsigned int wordlen=0;

		for (; 0!=(ch0=scanner.chr(0)); wordlen=scanner.wordlen(),scanner.skip())
		{
			if (ch0 == '.')
			{
				textwolf::UChar ch1 = scanner.chr(1);
				if (isDigit( ch1))
				{
					// dot in a number belongs to the number
					continue;
				}
				if (isDigit( ch1) || wordlen == 1)
				{
					// single characters followed by a dot.
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
							if (wordlen <= 3
							|| (isConsonant( ch3) && ch3 != ch2))
							{
								continue;
							}
						}
					}
					else
					{
						if (isConsonant( ch1) && isConsonant( ch2))
						{
							continue;
						}
					}
					if (ch1 != 'f' || ch2 != 'o' || ch3 != 'r')
					{
						// special case "Prof."
						continue;
					}
					if (ch1 == 'c' && wordlen <= 4)
					{
						// c at the end of a small word
						continue;
					}
					if (ch1 == 'z' && isConsonant( ch2))
					{
						// z after another consonant at the end a word
						continue;
					}
					if (ch1 == 'w' && isConsonant( ch2))
					{
						// w after another consonant at the end a word
						continue;
					}
					if (ch1 == 'v' && isConsonant( ch2))
					{
						// v after another consonant at the end a word
						continue;
					}
					if (ch1 == 'h' && ch2 != 'c' && isConsonant( ch2))
					{
						// h after a consonant other than 'c' at the end a word
						continue;
					}
					if (ch1 == 'k' && ch2 != 'c' && ch2 != 'l' && ch2 != 'n' && ch2 != 'r' && isConsonant( ch2))
					{
						// k after a consonant other than 'c','l','n' or 'r' at the end a word
						continue;
					}
				}
				rt.push_back( Position( scanner.pos(), 1));
			}
			else if (isPunctuation(ch0))
			{
				rt.push_back( Position( scanner.pos(), 1));
			}
		}
		return rt;
	}
};

const TokenizerInterface* strus::punctuationTokenizer_de()
{
	static const PunctuationTokenizer_de tokenizer;
	return &tokenizer;
}


