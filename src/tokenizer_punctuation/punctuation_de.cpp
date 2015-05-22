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
#include "punctuation_de.hpp"
#include "punctuation_utils.hpp"
#include <iostream>

using namespace strus;

#undef STRUS_LOWLEVEL_DEBUG

std::vector<analyzer::Token>
	PunctuationTokenizerExecutionContext_de::tokenize(
		const char* src, std::size_t srcsize)
{
	std::vector<analyzer::Token> rt;

	textwolf::UChar ch0;
	CharWindow scanner( src, srcsize, m_punctuation_char);
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
			rt.push_back( analyzer::Token( scanner.pos(), scanner.pos(), 1));
		}
		else if (isPunctuation(ch0))
		{
			rt.push_back( analyzer::Token( scanner.pos(), scanner.pos(), 1));
		}
	}
	return rt;
}


