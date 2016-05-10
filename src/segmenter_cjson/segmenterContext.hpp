/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef _STRUS_SEGMENTER_CONTEXT_CJSON_HPP_INCLUDED
#define _STRUS_SEGMENTER_CONTEXT_CJSON_HPP_INCLUDED
#include "strus/segmenterContextInterface.hpp"
#include "strus/errorBufferInterface.hpp"
#include "cjson/cJSON.h"
#include "private/xpathAutomaton.hpp"
#include <cstdlib>
#include <setjmp.h>

namespace strus
{

class SegmenterContext
	:public SegmenterContextInterface
{
public:
	explicit SegmenterContext( ErrorBufferInterface* errorhnd, const XPathAutomaton* automaton_)
		:m_automaton(automaton_)
		,m_content()
		,m_eof(false)
		,m_itemidx(0)
		,m_errorhnd(errorhnd)
	{}

	virtual ~SegmenterContext()
	{}

	virtual void putInput( const char* chunk, std::size_t chunksize, bool eof);

	virtual bool getNext( int& id, SegmenterPosition& pos, const char*& segment, std::size_t& segmentsize);

private:
	const XPathAutomaton* m_automaton;
	std::string m_content;
	bool m_eof;

	struct Item
	{
		int id;
		SegmenterPosition pos;
		const char* segment;
		std::size_t segmentsize;

		Item( int id_, SegmenterPosition pos_, const char* segment_, std::size_t segmentsize_)
			:id(id_),pos(pos_),segment(segment_),segmentsize(segmentsize_){}
		Item( const Item& o)
			:id(o.id),pos(o.pos),segment(o.segment),segmentsize(o.segmentsize){}
	};
	std::vector<Item> m_itemar;
	std::size_t m_itemidx;
	ErrorBufferInterface* m_errorhnd;	///< error buffer interface
};

}//namespace
#endif


