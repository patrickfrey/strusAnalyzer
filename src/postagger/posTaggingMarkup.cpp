/*
* Copyright (c) 2018 Patrick P. Frey
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/.
/*
* Copyright (c) 2018 Patrick P. Frey
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
/// \brief Implements XML markup of a content with POS tagging according to a fixed description
/// \file posTaggingMarkup.hpp
#include "posTaggingMarkup.hpp"
#include <string>
#include <vector>
#include <utility>
#include "private/internationalization.hpp"
#include "strus/base/utf8.hpp"
#include "strus/errorBufferInterface.hpp"

using namespace strus;


static std::string parseString( char const*& si, const char* se)
{
	std::string rt;
	char eb = *si;
	for (++si; si != se && *si != eb; ++si)
	{
		if (*si == '\n')
		{
			if (si == se) throw std::runtime_error(_TXT("string not terminated"));
		}
		if (*si == '\\')
		{
			++si;
			if (si == se) throw std::runtime_error(_TXT("string not terminated"));
		}
	}
	if (si == se) throw std::runtime_error(_TXT("string not terminated"));
	rt.append( si, se-si);
	++si;
	return rt;
}

static std::string parseToken( char const*& si, const char* se)
{
	for (++si; si != se && ((unsigned char)*si) >= 32; ++si){}
	return std::string( si, se-si);
}

std::vector<std::string> PosTaggingMarkup::tokenize( const char* src, int srclen)
{
	std::vector<std::string> rt;
	char const* si = src;
	const char* se = src + srclen;
	while (si < se)
	{
		if ((unsigned char)*si <= 32)
		{
			++si;
		}
		else if (*si == '\"' || *si == '\'')
		{
			rt.push_back( parseString( si, se));
		}
		else
		{
			rt.push_back( parseToken( si, se));
		}
	}
}

std::string PosTaggingMarkup::processContent( const char* src, int srclen) const
{
	std::string rt;
	std::vector<std::string> tokens( tokenize( src, srclen));
	std::vector<const char*> attlist = getTagList( tokens);
	std::vector<std::string>::const_iterator ti = tokens.begin(), te = tokens.end();
	std::vector<const char*>::const_iterator ai = attlist.begin(), ae = attlist.end();

	int tidx=0;
	for (; ti != te && ai != ae; ++ai,++ti,++tidx)
	{
		if (tidx) rt.push_back(' ');
		if (*ai)
		{
			rt.push_back( '<');
			rt.append( *ai);
			rt.push_back( '>');
			rt.append( *ti);
			rt.push_back( '<');
			rt.push_back( '/');
			rt.append( *ai);
			rt.push_back( '>');
		}
		else
		{
			rt.append( *ti);
		}
	}
	return rt;
}

void PosTaggingMarkup::addInfo( const std::vector<Element>& sentence)
{
	
}

std::string PosTaggingMarkup::process( const std::string& xmlcontent)
{
	
}


