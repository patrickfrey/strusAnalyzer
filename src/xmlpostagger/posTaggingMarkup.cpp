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

int PosTaggingMarkup::getOrCreateTagIdx( const std::string& tagnam)
{
	TagMap::const_iterator mi = m_tagmap.find( tagnam);
	if (mi == m_tagmap.end())
	{
		m_tagar.push_back( tagnam);
		return m_tagmap[ tagnam] = m_tagar.size();
	}
	else
	{
		return mi->second;
	}
}

int PosTaggingMarkup::getTagIdx( const std::string& tagnam) const
{
	TagMap::const_iterator mi = m_tagmap.find( tagnam);
	if (mi == m_tagmap.end()) throw strus::runtime_error(_TXT("tag name not defined in POS tagging table"));
	return mi->second;
}

int PosTaggingMarkup::getOrCreateValueIdx( const std::string& val)
{
	int rt = m_symtab.getOrCreate( val);
	if (!rt) throw std::runtime_error( m_errorhnd->fetchError());
	return rt;
}

int PosTaggingMarkup::getValueIdx( const std::string& val) const
{
	int rt = m_symtab.get( val);
	if (!rt) throw strus::runtime_error(_TXT("value not defined in POS tagging table"));
	return rt;
}

const std::string& PosTaggingMarkup::getTagName( int tag) const
{
	return m_tagar[ tag-1];
}

const char* PosTaggingMarkup::getValue( int value) const
{
	return m_symtab.key( value);
}

const char* PosTaggingMarkup::getTypeList( const std::vector<Element>& sentence)
{
	std::string buf;
	std::vector<Element>::const_iterator ei = sentence.begin(), ee = sentence.end();
	for (; ei != ee; ++ei)
	{
		char chrbuf[ 16];
		std::size_t len = strus::utf8encode( chrbuf, getOrCreateTagIdx( ei->type));
		buf.append( chrbuf, len);
	}
	const char* rt = m_allocator.allocStringCopy( buf);
	if (!rt) throw std::runtime_error( m_errorhnd->fetchError());
	return rt;
}

const char* PosTaggingMarkup::getValueList( const std::vector<Element>& sentence)
{
	std::string buf;
	std::vector<Element>::const_iterator ei = sentence.begin(), ee = sentence.end();
	for (; ei != ee; ++ei)
	{
		char chrbuf[ 16];
		std::size_t len = strus::utf8encode( chrbuf, getOrCreateValueIdx( ei->value));
		buf.append( chrbuf, len);
	}
	int kidx = m_seqtab.getOrCreate( buf);
	if (!kidx) throw std::runtime_error( m_errorhnd->fetchError());
	return m_seqtab.key(kidx);
}

std::vector<const char*> PosTaggingMarkup::getTagList( const std::vector<std::string>& sentence) const
{
	std::vector<const char*> rt;
	std::string buf;
	std::vector<std::string>::const_iterator ei = sentence.begin(), ee = sentence.end();
	for (; ei != ee; ++ei)
	{
		char chrbuf[ 16];
		std::size_t len = strus::utf8encode( chrbuf, getValueIdx( *ei));
		buf.append( chrbuf, len);
	}
	int kidx = m_seqtab.get( buf);
	if (!kidx) throw std::runtime_error( _TXT("sequence not found in POS tagging map"));
	char const* seq = m_seqtab.key( kidx);
	while (*seq)
	{
		int len = strus::utf8charlen( *seq);
		int chr = utf8decode( seq, len);
		rt.push_back( m_symtab.key( chr));
	}
	return rt;
}

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


