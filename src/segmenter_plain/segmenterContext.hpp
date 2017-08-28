/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef _STRUS_SEGMENTER_CONTEXT_PLAIN_HPP_INCLUDED
#define _STRUS_SEGMENTER_CONTEXT_PLAIN_HPP_INCLUDED
#include "strus/segmenterContextInterface.hpp"
#include "strus/errorBufferInterface.hpp"
#include "strus/reference.hpp"
#include "private/textEncoder.hpp"
#include <cstdlib>
#include <set>
#include <setjmp.h>

namespace strus
{

class SegmenterContext
	:public SegmenterContextInterface
{
public:
	explicit SegmenterContext( ErrorBufferInterface* errorhnd, const std::set<int>* ids_, const strus::Reference<strus::utils::TextEncoderBase>& encoder_)
		:m_ids(ids_)
		,m_content()
		,m_eof(false)
		,m_encoder(encoder_)
		,m_iditr(ids_->begin())
		,m_errorhnd(errorhnd)
	{}

	virtual ~SegmenterContext()
	{}

	virtual void putInput( const char* chunk, std::size_t chunksize, bool eof);

	virtual bool getNext( int& id, SegmenterPosition& pos, const char*& segment, std::size_t& segmentsize);

private:
	const std::set<int>* m_ids;
	std::string m_content;
	bool m_eof;
	strus::Reference<strus::utils::TextEncoderBase> m_encoder;

	std::set<int>::const_iterator m_iditr;
	ErrorBufferInterface* m_errorhnd;	///< error buffer interface
};

}//namespace
#endif


