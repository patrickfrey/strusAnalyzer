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
#include "strus/tokenMinerLib.hpp"
#include "strus/tokenMinerFactory.hpp"
#include "strus/tokenMiner.hpp"
#include "strus/analyzerInterface.hpp"
#include "strus/normalizerInterface.hpp"
#include "strus/tokenizerInterface.hpp"
#include "snowball.hpp"
#include "punctuation.hpp"
#include "chartable.hpp"
#include "dll_tags.hpp"
#include <boost/algorithm/string.hpp>

using namespace strus;
using namespace strus::tokenizer;

class WordSeparationTokenizer
	:public TokenizerInterface
{
public:
	WordSeparationTokenizer(){}

	virtual std::vector<Position> tokenize( const char* src, std::size_t srcsize) const
	{
		static const CharTable delimiter(":.;,!?%/()+-'\"`=");
		std::vector<Position> rt;
		std::size_t lastPos=0;
		std::size_t ii=0;
		for (;ii<srcsize; ++ii)
		{
			if ((unsigned char)src[ii] <= 32 || delimiter[ src[ii]])
			{
				if (ii > lastPos)
				{
					rt.push_back( Position( lastPos, ii-lastPos));
				}
				lastPos = ii+1;
			}
		}
		if (ii > lastPos)
		{
			rt.push_back( Position( lastPos, ii-lastPos));
		}
		return rt;
	}
};

static const WordSeparationTokenizer wordSeparationTokenizer;


class WhiteSpaceTokenizer
	:public TokenizerInterface
{
public:
	WhiteSpaceTokenizer(){}

	virtual std::vector<Position> tokenize( const char* src, std::size_t srcsize) const
	{
		std::vector<Position> rt;
		std::size_t lastPos=0;
		std::size_t ii=0;
		for (;ii<srcsize; ++ii)
		{
			if ((unsigned char)src[ii] <= 32)
			{
				if (ii > lastPos)
				{
					rt.push_back( Position( lastPos, ii-lastPos));
				}
				lastPos = ii+1;
			}
		}
		if (ii > lastPos)
		{
			rt.push_back( Position( lastPos, ii-lastPos));
		}
		return rt;
	}
};

static const WhiteSpaceTokenizer whiteSpaceTokenizer;



class EmptyNormalizer
	:public NormalizerInterface
{
public:
	EmptyNormalizer(){}

	virtual std::string normalize( const char* src, std::size_t srcsize) const
	{
		return std::string();
	}
};

EmptyNormalizer emptyNormalizer;



static const TokenMiner stem_de( &wordSeparationTokenizer, snowball_stemmer_de());
static const TokenMiner stem_dk( &wordSeparationTokenizer, snowball_stemmer_dk());
static const TokenMiner stem_nl( &wordSeparationTokenizer, snowball_stemmer_nl());
static const TokenMiner stem_en( &wordSeparationTokenizer, snowball_stemmer_en());
static const TokenMiner stem_fi( &wordSeparationTokenizer, snowball_stemmer_fi());
static const TokenMiner stem_fr( &wordSeparationTokenizer, snowball_stemmer_fr());
static const TokenMiner stem_hu( &wordSeparationTokenizer, snowball_stemmer_hu());
static const TokenMiner stem_it( &wordSeparationTokenizer, snowball_stemmer_it());
static const TokenMiner stem_no( &wordSeparationTokenizer, snowball_stemmer_no());
static const TokenMiner stem_ro( &wordSeparationTokenizer, snowball_stemmer_ro());
static const TokenMiner stem_ru( &wordSeparationTokenizer, snowball_stemmer_ru());
static const TokenMiner stem_pt( &wordSeparationTokenizer, snowball_stemmer_pt());
static const TokenMiner stem_es( &wordSeparationTokenizer, snowball_stemmer_es());
static const TokenMiner stem_se( &wordSeparationTokenizer, snowball_stemmer_se());
static const TokenMiner stem_tr( &wordSeparationTokenizer, snowball_stemmer_tr());
static const TokenMiner origword( &whiteSpaceTokenizer, 0);
static const TokenMiner origcontent( 0, 0);
static const TokenMiner punctuation_de( punctuationTokenizer_de(), 0);
static const TokenMiner emptyword( 0, &emptyNormalizer);


class TokenMinerFactoryImpl
	:public TokenMinerFactory
{
public:
	TokenMinerFactoryImpl( const std::string&){}

	virtual const TokenMiner* get( const std::string& name) const
	{
		if (boost::iequals( name, "stem_de")) return &stem_de;
		else if (boost::iequals( name, "stem_dk")) return &stem_dk;
		else if (boost::iequals( name, "stem_nl")) return &stem_nl;
		else if (boost::iequals( name, "stem_en")) return &stem_en;
		else if (boost::iequals( name, "stem_fi")) return &stem_fi;
		else if (boost::iequals( name, "stem_fr")) return &stem_fr;
		else if (boost::iequals( name, "stem_hu")) return &stem_hu;
		else if (boost::iequals( name, "stem_it")) return &stem_it;
		else if (boost::iequals( name, "stem_no")) return &stem_no;
		else if (boost::iequals( name, "stem_ro")) return &stem_ro;
		else if (boost::iequals( name, "stem_ru")) return &stem_ru;
		else if (boost::iequals( name, "stem_pt")) return &stem_pt;
		else if (boost::iequals( name, "stem_es")) return &stem_es;
		else if (boost::iequals( name, "stem_se")) return &stem_se;
		else if (boost::iequals( name, "stem_tr")) return &stem_tr;
		else if (boost::iequals( name, "origword")) return &origword;
		else if (boost::iequals( name, "origcontent")) return &origcontent;
		else if (boost::iequals( name, "punctuation_de")) return &punctuation_de;
		else if (boost::iequals( name, "empty")) return &emptyword;
		return 0;
	}
};

DLL_PUBLIC strus::TokenMinerFactory*
	strus::createTokenMinerFactory( const std::string& source)
{
	return new TokenMinerFactoryImpl( source);
}


