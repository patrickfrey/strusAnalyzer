/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef _STRUS_SEGMENTER_MARKUP_CONTEXT_TEXTWOLF_HPP_INCLUDED
#define _STRUS_SEGMENTER_MARKUP_CONTEXT_TEXTWOLF_HPP_INCLUDED
#include "strus/segmenterMarkupContextInterface.hpp"
#include "strus/errorBufferInterface.hpp"
#include "private/errorUtils.hpp"
#include "private/internationalization.hpp"
#include "segmenter.hpp"
#include "private/xpathAutomaton.hpp"
#include "textwolf/charset.hpp"
#include "textwolf/sourceiterator.hpp"
#include <cstdlib>
#include <setjmp.h>

namespace strus
{

template <class CharsetEncoding>
class SegmenterMarkupContext
	:public SegmenterMarkupContextInterface
{
public:
	explicit SegmenterMarkupContext( ErrorBufferInterface* errorhnd_, const std::string& source_, const CharsetEncoding& charset_=CharsetEncoding())
		:m_source(source_)
		,m_scanner()
		,m_taglevel(1)
		,m_eof(false)
		,m_errorhnd(errorhnd_)
	{
		m_scanner.setSource( textwolf::SrcIterator( m_source.c_str(), m_source.size()));
		m_itr = m_scanner.begin(false);
		m_end = m_scanner.end();
	}

	virtual ~SegmenterMarkupContext()
	{}

	virtual int getNext( SegmenterPosition& segpos, const char*& segment, std::size_t& segmentsize)
	{
		try
		{
			if (m_taglevel == 0) return 0;
			++m_itr;
			if (m_itr == m_end) return 0;

			typename XMLScanner::ElementType et = m_itr->type();
			if (et == XMLScanner::ErrorOccurred)
			{
				const char* errstr = "";
				m_scanner.getError( &errstr);
				m_errorhnd->report( "error in XML document at position %u: %s", (unsigned int)m_scanner.getTokenPosition(), errstr);
				return false;
			}
			else if (et == XMLScanner::Exit)
			{
				return m_taglevel=0;
			}
			segment = m_itr->content();
			segmentsize = m_itr->size();
			return m_taglevel;
		}
		CATCH_ERROR_MAP_ARG1_RETURN( _TXT("error in get next for markup context of '%s' segmenter: %s"), "textwolf", *m_errorhnd, 0);
	}

	struct MarkupElement
	{
		std::size_t segpos;
		std::size_t pos;
		std::size_t idx;
		std::string marker;

		MarkupElement( std::size_t segpos_, std::size_t pos_, std::size_t idx_, std::string marker_)
			:segpos(segpos_),pos(pos_),idx(idx_),marker(marker_){}
		MarkupElement( const MarkupElement& o)
			:segpos(o.segpos),pos(o.pos),idx(o.idx),marker(o.marker){}

		bool operator < (const MarkupElement& o) const
		{
			if (segpos < o.segpos) return true;
			if (segpos > o.segpos) return false;
			if (pos < o.pos) return true;
			if (pos > o.pos) return false;
			if (idx < o.idx) return true;
			if (idx > o.idx) return false;
			if (marker < o.marker) return true;
			if (marker > o.marker) return false;
			return false;
		}
	};

	virtual void putMarkup(
			std::size_t segpos,
			std::size_t pos,
			const std::string& marker)
	{
		try
		{
			m_markups.push_back( MarkupElement( segpos, pos, m_markups.size(), marker));
		}
		CATCH_ERROR_MAP_ARG1( _TXT("error in put markup of '%s' segmenter markup context: %s"), "textwolf", *m_errorhnd);
	}

	virtual std::string getContent() const
	{
		try
		{
			std::vector<MarkupElement> markups = m_markups;
			std::sort( markups.begin(), markups.end());
			std::string rt;
			return rt;
		}
		CATCH_ERROR_MAP_ARG1_RETURN( _TXT("error in get next for markup context of '%s' segmenter: %s"), "textwolf", *m_errorhnd, std::string());
	}

private:
	typedef textwolf::XMLScanner<
			textwolf::SrcIterator,
			CharsetEncoding,
			textwolf::charset::UTF8,
			std::string
		> XMLScanner;

	std::string m_source;
	XMLScanner m_scanner;
	typename XMLScanner::iterator m_itr;
	typename XMLScanner::iterator m_end;
	int m_taglevel;
	bool m_eof;
	jmp_buf m_eom;
	std::vector<MarkupElement> m_markups;
	ErrorBufferInterface* m_errorhnd;
};

}//namespace
#endif


