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
#include "strus/reference.hpp"
#include "private/textEncoder.hpp"
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
	explicit SegmenterContext( ErrorBufferInterface* errorhnd, const XPathAutomaton* automaton_, const strus::Reference<strus::utils::TextEncoderBase>& encoder_)
		:m_automaton(automaton_)
		,m_content()
		,m_tree(0)
		,m_eof(false)
		,m_encoder(encoder_)
		,m_itemidx(0)
		,m_errorhnd(errorhnd)
	{}

	virtual ~SegmenterContext()
	{
		if (m_tree) cJSON_Delete( m_tree);
	}

	virtual void putInput( const char* chunk, std::size_t chunksize, bool eof);

	virtual bool getNext( int& id, SegmenterPosition& pos, const char*& segment, std::size_t& segmentsize);

public:
	struct Item
	{
		int id;
		SegmenterPosition pos;
		const char* segment;
		std::size_t segmentsize;

#if __cplusplus >= 201103L
		Item( Item&& ) = default;
		Item( const Item& ) = default;
		Item& operator= ( Item&& ) = default;
		Item& operator= ( const Item& ) = default;
#else
		Item( const Item& o)
			:id(o.id),pos(o.pos),segment(o.segment),segmentsize(o.segmentsize){}
#endif
		Item( int id_, SegmenterPosition pos_, const char* segment_, std::size_t segmentsize_)
			:id(id_),pos(pos_),segment(segment_),segmentsize(segmentsize_){}
	};

private:
	const XPathAutomaton* m_automaton;
	std::string m_content;
	cJSON* m_tree;
	bool m_eof;
	strus::Reference<strus::utils::TextEncoderBase> m_encoder;

	std::vector<Item> m_itemar;
	std::size_t m_itemidx;
	ErrorBufferInterface* m_errorhnd;	///< error buffer interface
};

}//namespace
#endif


