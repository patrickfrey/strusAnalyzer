/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "punctuation_de.hpp"
#include "punctuation_utils.hpp"
#include "private/errorUtils.hpp"
#include "private/internationalization.hpp"
#include "strus/errorBufferInterface.hpp"
#include "strus/base/introspection.hpp"
#include <iostream>

using namespace strus;

#undef STRUS_LOWLEVEL_DEBUG

std::vector<analyzer::Token>
	PunctuationTokenizerInstance_de::tokenize(
		const char* src, std::size_t srcsize) const
{
	try
	{
		std::vector<analyzer::Token> rt;
	
		textwolf::UChar ch0;
		CharWindow scanner( src, srcsize, &m_punctuation_char);
		unsigned int wordlen=0;
		unsigned int pos = 0;
	
		for (; 0!=(ch0=scanner.chr(0)); wordlen=scanner.wordlen(),scanner.skip())
		{
			if (ch0 == '-')
			{
				textwolf::UChar ch1 = scanner.chr(1);
				if (isAlpha( ch1)) continue;
			}
			else if (ch0 == '.')
			{
				pos = scanner.itrpos();
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
				if (isLowercase( ch1))
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
				rt.push_back( analyzer::Token( pos/*ordpos*/, 0/*seg*/, pos, 1));
#ifdef STRUS_LOWLEVEL_DEBUG
				std::cout << "PUNKT " << (int)__LINE__ << ":" << scanner.tostring() << std::endl;
				std::size_t endpos = pos;
				std::size_t startpos = (endpos > 16)?(endpos-16):0;
				std::cout << "TOKEN AT " << std::string( src+startpos, endpos-startpos) << std::endl;
#endif
			}
			else if (isPunctuation(ch0))
			{
				pos = scanner.itrpos();
				rt.push_back( analyzer::Token( pos/*ordpos*/, 0, pos, 1));
#ifdef STRUS_LOWLEVEL_DEBUG
				std::size_t endpos = pos;
				std::size_t startpos = (endpos > 16)?(endpos-16):0;
				std::cout << "TOKEN AT " << std::string( src+startpos, endpos-startpos) << std::endl;
#endif
			}
		}
		return rt;
	}
	CATCH_ERROR_MAP_RETURN( _TXT("error in 'punctuation' tokenizer: %s"), *m_errorhnd, std::vector<analyzer::Token>());
}


IntrospectionInterface* PunctuationTokenizerInstance_de::createIntrospection() const
{
	class Description :public StructTypeIntrospectionDescription<PunctuationTokenizerInstance_de>{
	public:
		Description()
		{
			(*this)
			( "charlist", &PunctuationTokenizerInstance_de::m_punctuation_charlist, AtomicTypeIntrospection<std::string>::constructor)
			;
		}
	};
	static const Description descr;
	try
	{
		return new StructTypeIntrospection<PunctuationTokenizerInstance_de>( this, &descr, m_errorhnd);
	}
	CATCH_ERROR_MAP_RETURN( _TXT("error creating introspection: %s"), *m_errorhnd, NULL);
}


