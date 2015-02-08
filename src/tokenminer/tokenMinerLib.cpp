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
	virtual Context* createContext() const
	{
		return 0;
	}
	virtual std::string normalize( Context*, const char* src, std::size_t srcsize) const
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
	TokenMinerFactoryImpl()
	{
		init();
	}

	virtual void define( const std::string& name, const TokenMiner* tokenMiner)
	{
		m_map[ boost::algorithm::to_lower_copy( name)] = tokenMiner;
	}

	virtual const TokenMiner* get( const std::string& name) const
	{
		std::map<std::string,const TokenMiner*>::const_iterator
			mi = m_map.find( boost::algorithm::to_lower_copy( name));
		if (mi == m_map.end()) return 0;
		return mi->second;
	}

private:
	void init()
	{
		define( "stem_de", &stem_de);
		define( "stem_dk", &stem_dk);
		define( "stem_nl", &stem_nl);
		define( "stem_en", &stem_en);
		define( "stem_fi", &stem_fi);
		define( "stem_fr", &stem_fr);
		define( "stem_hu", &stem_hu);
		define( "stem_it", &stem_it);
		define( "stem_no", &stem_no);
		define( "stem_ro", &stem_ro);
		define( "stem_ru", &stem_ru);
		define( "stem_pt", &stem_pt);
		define( "stem_es", &stem_es);
		define( "stem_se", &stem_se);
		define( "stem_tr", &stem_tr);
		define( "origword", &origword);
		define( "origcontent", &origcontent);
		define( "punctuation_de", &punctuation_de);
		define( "empty", &emptyword);
	}

private:
	std::map<std::string,const TokenMiner*> m_map;
};


DLL_PUBLIC strus::TokenMinerFactory*
	strus::createTokenMinerFactory()
{
	return new TokenMinerFactoryImpl();
}


