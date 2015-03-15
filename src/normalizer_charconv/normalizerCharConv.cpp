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
#include "normalizerCharConv.hpp"
#include "textwolf/charset_utf8.hpp"
#include "textwolf/cstringiterator.hpp"
#include "private/utils.hpp"
#include <cstring>

using namespace strus;

void CharMap::set( unsigned int chr, const char* value)
{
	m_map[ chr] = m_strings.size();
	m_strings.append( value);
	m_strings.push_back( '\0');
}

void CharMap::set( unsigned int chr, unsigned int mapchr)
{
	textwolf::charset::UTF8 utf8;
	std::string buf;
	utf8.print( mapchr, buf);
	m_map[ chr] = m_strings.size();
	m_strings.append( buf);
	m_strings.push_back( '\0');
}

void CharMap::buildMapDiacritical( ConvType diatype)
{
	set( 0xC0, "A");
	set( 0xC1, "A");
	set( 0xC2, "A");
	set( 0xC3, "A");
	set( 0xC4, diatype==DiacriticalGerman?"Ae":"A");
	set( 0xC5, "A");
	set( 0xC6, "AE");

	set( 0xC7, "C");

	set( 0xC8, "E");
	set( 0xC9, "E");
	set( 0xCA, "E");
	set( 0xCB, "E");

	set( 0xCC, "I");
	set( 0xCD, "I");
	set( 0xCE, "I");
	set( 0xCF, "I");

	set( 0xD0, "Th");
	set( 0xD1, "N");

	set( 0xD2, "O");
	set( 0xD3, "O");
	set( 0xD4, "O");
	set( 0xD5, "O");

	set( 0xD6, diatype==DiacriticalGerman?"Oe":"O");
	set( 0xD7, "*");
	set( 0xD8, "O");

	set( 0xD8, "U");
	set( 0xD9, "U");
	set( 0xDA, "U");
	set( 0xDB, "U");
	set( 0xDC, diatype==DiacriticalGerman?"Ue":"U");

	set( 0xDD, "Y");
	set( 0xDE, "Th");
	set( 0xDF, "Ss");

	set( 0xE0, "a");
	set( 0xE1, "a");
	set( 0xE2, "a");
	set( 0xE3, "a");
	set( 0xE4, diatype==DiacriticalGerman?"ae":"a");
	set( 0xE5, "a");
	set( 0xE6, "ae");

	set( 0xE7, "c");

	set( 0xE8, "e");
	set( 0xE9, "e");
	set( 0xEA, "e");
	set( 0xEB, "e");

	set( 0xEC, "i");
	set( 0xED, "i");
	set( 0xEE, "i");
	set( 0xEF, "i");

	set( 0xF0, "th");
	set( 0xF1, "n");

	set( 0xF2, "o");
	set( 0xF3, "o");
	set( 0xF4, "o");
	set( 0xF5, "o");
	set( 0xF6, diatype==DiacriticalGerman?"oe":"o");

	set( 0xF7, "/");
	set( 0xF8, "o");

	set( 0xF9, "u");
	set( 0xFA, "u");
	set( 0xFB, "u");
	set( 0xFC, diatype==DiacriticalGerman?"ue":"u");

	set( 0xFD, "y");
	set( 0xFE, "th");
	set( 0xFF, "y");

	set( 0x102, "A");
	set( 0x103, "a");
	set( 0x218, "S");
	set( 0x15F, "S");
	set( 0x219, "s");
	set( 0x15E, "s");
	set( 0x21A, "T");
	set( 0x162, "T");
	set( 0x21B, "t");
	set( 0x163, "t");

	set( 0x1E9E, "ss");
}

void CharMap::buildMapTolower()
{
	for (char ch='A'; ch <= 'Z'; ++ch)
	{
		set( ch, ch|32);
	}
	set( 0xC0, 0xE0);
	set( 0xC1, 0xE1);
	set( 0xC2, 0xE2);
	set( 0xC3, 0xE3);
	set( 0xC4, 0xE4);
	set( 0xC5, 0xE5);
	set( 0xC6, 0xE6);

	set( 0xC7, 0xE7);

	set( 0xC8, 0xE8);
	set( 0xC9, 0xE9);
	set( 0xCA, 0xEA);
	set( 0xCB, 0xEB);

	set( 0xCC, 0xEC);
	set( 0xCD, 0xED);
	set( 0xCE, 0xEE);
	set( 0xCF, 0xEF);

	set( 0xD0, 0xF0);
	set( 0xD1, 0xF1);

	set( 0xD2, 0xF2);
	set( 0xD3, 0xF3);
	set( 0xD4, 0xF4);
	set( 0xD5, 0xF5);

	set( 0xD6, 0xF6);
	set( 0xD8, 0xF8);
	set( 0xD9, 0xF9);
	set( 0xDA, 0xFA);
	set( 0xDB, 0xFB);
	set( 0xDC, 0xFC);

	set( 0xDD, 0xFD);
	set( 0xDE, 0xFE);
	set( 0x9F, 0xFF);
	set( 0x1E9E, 0xDF);

	set( 0x102, 0x103);
	set( 0x218, 0x219);
	set( 0x15E, 0x15F);
	set( 0x21A, 0x21B);
	set( 0x162, 0x163);
}

void CharMap::buildMapToupper()
{
	for (char ch='A'; ch <= 'Z'; ++ch)
	{
		set( ch|32, ch);
	}
	set( 0xE0, 0xC0);
	set( 0xE1, 0xC1);
	set( 0xE2, 0xC2);
	set( 0xE3, 0xC3);
	set( 0xE4, 0xC4);
	set( 0xE5, 0xC5);
	set( 0xE6, 0xC6);

	set( 0xE7, 0xE7);

	set( 0xE8, 0xC8);
	set( 0xE9, 0xC9);
	set( 0xEA, 0xCA);
	set( 0xEB, 0xCB);

	set( 0xEC, 0xCC);
	set( 0xED, 0xCD);
	set( 0xEE, 0xCE);
	set( 0xEF, 0xCF);

	set( 0xF0, 0xD0);
	set( 0xF1, 0xD1);

	set( 0xF2, 0xD2);
	set( 0xF3, 0xD3);
	set( 0xF4, 0xD4);
	set( 0xF5, 0xD5);

	set( 0xF6, 0xD6);
	set( 0xF8, 0xD8);
	set( 0xF9, 0xD9);
	set( 0xFA, 0xDA);
	set( 0xFB, 0xDB);
	set( 0xFC, 0xDC);

	set( 0xFD, 0xDD);
	set( 0xFE, 0xDE);
	set( 0x9F, 0xDF);
	set( 0xDF, 0x1E9E);

	set( 0x103, 0x102);
	set( 0x219, 0x218);
	set( 0x15F, 0x15E);
	set( 0x21B, 0x21A);
	set( 0x163, 0x162);
}

void CharMap::load( ConvType type)
{
	switch (type)
	{
		case DiacriticalUnknown:
			buildMapDiacritical( DiacriticalUnknown);
			break;
		case DiacriticalGerman:
			buildMapDiacritical( DiacriticalGerman);
			break;
		case Lowercase:
			buildMapTolower();
			break;
		case Uppercase:
			buildMapToupper();
			break;
	}
}

std::string CharMap::rewrite( const char* src, std::size_t srcsize) const
{
	std::string rt;
	textwolf::charset::UTF8 utf8;
	char buf[16];
	unsigned int bufpos;
	textwolf::CStringIterator itr( src, srcsize);

	while (*itr)
	{
		bufpos = 0;
		textwolf::UChar value = utf8.value( buf, bufpos, itr);
		std::map<unsigned int,std::size_t>::const_iterator mi = m_map.find( value);
		if (mi == m_map.end())
		{
			rt.append( buf, bufpos);
		}
		else
		{
			rt.append( m_strings.c_str() + mi->second);
		}
	}
	return rt;
}

DiacriticalNormalizer::ThisArgument::ThisArgument( const std::string& language)
{
	std::string language_lo = utils::tolower( language);
	if (language_lo == "de")
	{
		m_map.load( CharMap::DiacriticalGerman);
	}
	else
	{
		m_map.load( CharMap::DiacriticalUnknown);
	}
}

NormalizerInterface::Argument* DiacriticalNormalizer::createArgument( const std::vector<std::string>& arg) const
{
	if (arg.size() != 1)
	{
		throw std::runtime_error( "illegal number of arguments passed to snowball stemmer");
	}
	return new ThisArgument( arg[0]);
}

NormalizerInterface::Context* DiacriticalNormalizer::createContext( const Argument* arg) const
{
	return new ThisContext( reinterpret_cast<const ThisArgument*>( arg));
}


