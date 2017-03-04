/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "segmenterContext.hpp"
#include "strus/errorBufferInterface.hpp"
#include "segmenter.hpp"
#include "private/xpathAutomaton.hpp"
#include "private/utils.hpp"
#include "private/errorUtils.hpp"
#include "private/internationalization.hpp"
#include "textwolf/xmlscanner.hpp"
#include <cstdlib>
#include <cstring>
#include <setjmp.h>
#undef STRUS_LOWLEVEL_DEBUG
#ifdef STRUS_LOWLEVEL_DEBUG
#include <iostream>
#endif

using namespace strus;

void SegmenterContext::putInput( const char* chunk, std::size_t chunksize, bool eof)
{
	try
	{
		m_content.append( chunk, chunksize);
		m_eof |= eof;
	}
	CATCH_ERROR_MAP( _TXT("error in put input of JSON segmenter: %s"), *m_errorhnd);
}

struct TextwolfItem
{
	typedef textwolf::XMLScannerBase::ElementType Type;
	Type type;
	const char* value;

	TextwolfItem( const TextwolfItem& o)
		:type(o.type),value(o.value){}
	explicit TextwolfItem( const Type& type_, const char* value_=0)
		:type(type_),value(value_){}
};


static void getTextwolfValue( std::vector<TextwolfItem>& tiar, cJSON const* nd, const char* value)
{
	typedef textwolf::XMLScannerBase TX;
	if (nd->string)
	{
		if (nd->string[0] == '-')
		{
			tiar.push_back( TextwolfItem( TX::TagAttribName, nd->string+1));
			tiar.push_back( TextwolfItem( TX::TagAttribValue, value));
		}
		else if (nd->string[0] == '#' && std::strcmp( nd->string, "#text") == 0)
		{
			tiar.push_back( TextwolfItem( TX::Content, value));
		}
		else
		{
			tiar.push_back( TextwolfItem( TX::OpenTag, nd->string));
			tiar.push_back( TextwolfItem( TX::Content, value));
			tiar.push_back( TextwolfItem( TX::CloseTag, nd->string));
		}
	}
	else
	{
		tiar.push_back( TextwolfItem( TX::Content, value));
	}
}

static void getTextwolfItems( std::vector<TextwolfItem>& itemar, cJSON const* nd)
{
	typedef textwolf::XMLScannerBase TX;
	switch (nd->type & 0x7F)
	{
		case cJSON_False:
			getTextwolfValue( itemar, nd, "false");
			break;
		case cJSON_True:
			getTextwolfValue( itemar, nd, "true");
			break;
		case cJSON_NULL:
			if (nd->string && nd->string[0] != '-' && nd->string[0] != '#')
			{
				itemar.push_back( TextwolfItem( TX::OpenTag, nd->string));
				itemar.push_back( TextwolfItem( TX::CloseTagIm));
			}
			break;
		case cJSON_String:
			getTextwolfValue( itemar, nd, nd->valuestring);
			break;
		case cJSON_Number:
			if (!nd->valuestring)
			{
				throw strus::runtime_error( _TXT("value node without string value found in JSON structure"));
			}
			getTextwolfValue( itemar, nd, nd->valuestring);
			break;
		case cJSON_Array:
		{
			cJSON const* chnd = nd->child;
			if (nd->string)
			{
				for (;chnd; chnd = chnd->next)
				{
					itemar.push_back( TextwolfItem( TX::OpenTag, nd->string));
					getTextwolfItems( itemar, chnd);
					itemar.push_back( TextwolfItem( TX::CloseTag, nd->string));
				}
			}
			else
			{
				unsigned int idx=0;
				char idxstr[ 64];
				for (;chnd; chnd = chnd->next)
				{
					snprintf( idxstr, sizeof( idxstr), "%u", idx);
					itemar.push_back( TextwolfItem( TX::OpenTag, idxstr));
					getTextwolfItems( itemar, chnd);
					itemar.push_back( TextwolfItem( TX::CloseTag, idxstr));
				}
			}
			break;
		}
		case cJSON_Object:
		{
			cJSON const* chnd = nd->child;
			if (nd->string)
			{
				itemar.push_back( TextwolfItem( TX::OpenTag, nd->string));
				for (;chnd; chnd = chnd->next)
				{
					getTextwolfItems( itemar, chnd);
				}
				itemar.push_back( TextwolfItem( TX::CloseTag, nd->string));
			}
			else
			{
				for (;chnd; chnd = chnd->next)
				{
					getTextwolfItems( itemar, chnd);
				}
			}
			break;
		}
		default:
			throw std::runtime_error( "internal: memory corruption found in JSON structure");
	}
}

static std::pair<std::size_t,std::size_t> lineInfo( const char* begin, const char* at)
{
	std::size_t line = 1, col = 1;
	char const* si = begin;
	for (; si < at; ++si)
	{
		if (*si == '\n')
		{
			++line;
			col=1;
		}
		else
		{
			++col;
		}
	}
	return std::pair<std::size_t,std::size_t>(line,col);
}

static cJSON* parseJsonTree( const std::string& content)
{
	cJSON_Context ctx;
	cJSON* tree = cJSON_Parse( &ctx, content.c_str());
	if (!tree)
	{
		if (!ctx.errorptr) throw std::bad_alloc();
		std::pair<std::size_t,std::size_t> li = lineInfo( content.c_str(), ctx.errorptr);

		std::string err( ctx.errorptr);
		throw strus::runtime_error( _TXT( "error in JSON at line %u, column %u: %s"), li.first, li.second, err.c_str());
	}
	return tree;
}

static void getSegmenterItems( const XPathAutomaton* automaton, std::vector<SegmenterContext::Item>& resar, const cJSON* tree)
{
	if (!tree) return;

	XPathAutomatonContext xpathselect( automaton->createContext());
	std::vector<TextwolfItem> itemar;
	getTextwolfItems( itemar, tree);
	SegmenterPosition pos = 0;
	std::vector<TextwolfItem>::const_iterator ti = itemar.begin(), te = itemar.end();
	for (; ti != te; ++ti)
	{
#ifdef STRUS_LOWLEVEL_DEBUG
		const char* elemtypenam = textwolf::XMLScannerBase::getElementTypeName(ti->type);
		std::cout << "input " << elemtypenam << " " << ti->value << std::endl;
#endif
		std::size_t valuesize = ti->value?std::strlen(ti->value):0;
		xpathselect.putElement( ti->type, ti->value, valuesize);
		int segid;
		while (xpathselect.getNext( segid))
		{
			resar.push_back( SegmenterContext::Item( segid, pos, ti->value, valuesize));
		}
		pos += valuesize+1;
	}
}

bool SegmenterContext::getNext( int& id, SegmenterPosition& pos, const char*& segment, std::size_t& segmentsize)
{
	try
	{
	AGAIN:
		if (m_itemidx < m_itemar.size())
		{
			const Item& curitem = m_itemar[ m_itemidx];
			id = curitem.id;
			pos = curitem.pos;
			segment = curitem.segment;
			segmentsize = curitem.segmentsize;
			++m_itemidx;
			return true;
		}
		else if (m_itemidx == m_itemar.size())
		{
			if (!m_tree)
			{
				m_tree = parseJsonTree( m_content);
				getSegmenterItems( m_automaton, m_itemar, m_tree);
#ifdef STRUS_LOWLEVEL_DEBUG
				std::cout << "got " << m_itemar.size() << " items" << std::endl;
				std::vector<Item>::const_iterator ti = m_itemar.begin(), te = m_itemar.end();
				for (; ti != te; ++ti)
				{
					std::cout << "item " << ti->id << " at " << ti->pos << " " << std::string( ti->segment, ti->segmentsize) << std::endl;
				}
#endif
				goto AGAIN;
			}
			return false;
		}
		else
		{
			return false;
		}
	}
	CATCH_ERROR_MAP_RETURN( _TXT("error in JSON segmenter get next: %s"), *m_errorhnd, false);
}

