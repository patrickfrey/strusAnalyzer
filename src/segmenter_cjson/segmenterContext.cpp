/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "segmenterContext.hpp"
#include "cjson2textwolf.hpp"
#include "strus/errorBufferInterface.hpp"
#include "segmenter.hpp"
#include "private/xpathAutomaton.hpp"
#include "private/errorUtils.hpp"
#include "private/internationalization.hpp"
#include <cstdlib>
#include <cstring>
#include <setjmp.h>
#undef STRUS_LOWLEVEL_DEBUG
#ifdef STRUS_LOWLEVEL_DEBUG
#include <iostream>
#endif

using namespace strus;

void JsonSegmenterContext::putInput( const char* chunk, std::size_t chunksize, bool eof)
{
	try
	{
		m_content.append( chunk, chunksize);
		if (eof)
		{
			m_eof = true;
			if (m_encoder.get())
			{
				m_content = m_encoder->convert( m_content.c_str(), m_content.size(), true);
			}
		}
	}
	CATCH_ERROR_MAP( _TXT("error in put input of JSON segmenter: %s"), *m_errorhnd);
}

static void getSegmenterItems( const XPathAutomaton* automaton, std::vector<JsonSegmenterContext::Item>& resar, const cJSON* tree)
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
			resar.push_back( JsonSegmenterContext::Item( segid, pos, ti->value, valuesize));
		}
		if (ti->type != textwolf::XMLScannerBase::CloseTag)
		{
			pos += valuesize+1;
		}
		else
		{
			pos += 1;
		}
	}
}

bool JsonSegmenterContext::getNext( int& id, SegmenterPosition& pos, const char*& segment, std::size_t& segmentsize)
{
	try
	{
		if (!m_eof) return false;
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

