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
#include "dll_tags.hpp"
#include <boost/algorithm/string.hpp>

using namespace strus;

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
		return rt;
	}
};

static const WhiteSpaceTokenizer whiteSpaceTokenizer;


static const TokenMiner stem_de( &whiteSpaceTokenizer, snowball_stemmer_de());
static const TokenMiner stem_dk( &whiteSpaceTokenizer, snowball_stemmer_dk());
static const TokenMiner stem_nl( &whiteSpaceTokenizer, snowball_stemmer_nl());
static const TokenMiner stem_en( &whiteSpaceTokenizer, snowball_stemmer_en());
static const TokenMiner stem_fi( &whiteSpaceTokenizer, snowball_stemmer_fi());
static const TokenMiner stem_fr( &whiteSpaceTokenizer, snowball_stemmer_fr());
static const TokenMiner stem_it( &whiteSpaceTokenizer, snowball_stemmer_it());
static const TokenMiner stem_no( &whiteSpaceTokenizer, snowball_stemmer_no());
static const TokenMiner stem_pt( &whiteSpaceTokenizer, snowball_stemmer_pt());
static const TokenMiner stem_es( &whiteSpaceTokenizer, snowball_stemmer_es());
static const TokenMiner stem_se( &whiteSpaceTokenizer, snowball_stemmer_se());
static const TokenMiner origword( &whiteSpaceTokenizer, 0);


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
		else if (boost::iequals( name, "stem_it")) return &stem_it;
		else if (boost::iequals( name, "stem_no")) return &stem_no;
		else if (boost::iequals( name, "stem_pt")) return &stem_pt;
		else if (boost::iequals( name, "stem_es")) return &stem_es;
		else if (boost::iequals( name, "stem_se")) return &stem_se;
		else if (boost::iequals( name, "origword")) return &origword;
		return 0;
	}
};

DLL_PUBLIC strus::TokenMinerFactory*
	strus::createTokenMinerFactory( const std::string& source)
{
	return new TokenMinerFactoryImpl( source);
}


