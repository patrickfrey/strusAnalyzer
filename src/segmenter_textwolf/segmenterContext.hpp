/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef _STRUS_SEGMENTER_CONTEXT_TEXTWOLF_HPP_INCLUDED
#define _STRUS_SEGMENTER_CONTEXT_TEXTWOLF_HPP_INCLUDED
#include "strus/segmenterContextInterface.hpp"
#include "private/utils.hpp"
#include "private/internationalization.hpp"
#include "private/errorUtils.hpp"
#include "strus/errorBufferInterface.hpp"
#include "segmenter.hpp"
#include "private/xpathAutomaton.hpp"
#include "private/textEncoder.hpp"
#include "textwolf/charset.hpp"
#include "textwolf/sourceiterator.hpp"
#include <cstdlib>
#include <setjmp.h>

#define SEGMENTER_NAME "textwolf"

namespace strus
{

template <class CharsetEncoding>
class SegmenterContext
	:public SegmenterContextInterface
{
public:
	explicit SegmenterContext( ErrorBufferInterface* errorhnd, const XPathAutomaton* automaton_, const CharsetEncoding& charset_=CharsetEncoding())
		:m_automaton(automaton_)
		,m_xpathselect(automaton_->createContext())
		,m_scanner(charset_,textwolf::SrcIterator())
		,m_chunk(0)
		,m_chunksize(0)
		,m_eof(false)
		,m_got_exit(false)
		,m_chunkbuf()
		,m_errorhnd(errorhnd)
	{
	}

	virtual ~SegmenterContext()
	{}

	void initScanner( char const* src, std::size_t srcsize)
	{
		m_chunk = src;
		m_chunksize = srcsize;
		m_srciter.putInput( m_chunk, m_chunksize, &m_eom);
		m_scanner.setSource( m_srciter);
		m_itr = m_scanner.begin(false);
		m_end = m_scanner.end();
	}
	bool feedNextInputChunk()
	{
		if (m_chunkbuf.empty())
		{
			initScanner( "", 0);
			return false;
		}
		else
		{
			initScanner( m_chunkbuf.front().c_str(), m_chunkbuf.front().size());
			return true;
		}
	}

	virtual void putInput( const char* chunk, std::size_t chunksize, bool eof)
	{
		try
		{
			if (m_got_exit)
			{
				m_errorhnd->report( _TXT( "feeded chunk after parsing completed"));
				return;
			}
			if (m_eof)
			{
				m_errorhnd->report( _TXT( "feeded chunk after declared end of input"));
				return;
			}
			try
			{
				m_chunkbuf.push_back( std::string( chunk, chunksize));
				m_eof = eof;
			}
			catch (const std::bad_alloc&)
			{
				m_errorhnd->report( _TXT("out of memory when buffering chunks for '%s' segmenter"), SEGMENTER_NAME);
			}
			if (!m_chunk)
			{
				feedNextInputChunk();
			}
		}
		CATCH_ERROR_MAP_ARG1( _TXT("error in put input of '%s' segmenter: %s"), SEGMENTER_NAME, *m_errorhnd);
	}

	virtual bool getNext( int& id, SegmenterPosition& pos, const char*& segment, std::size_t& segmentsize)
	{
		try
		{
			if (m_got_exit) return false;

		AGAIN:
			if (setjmp(m_eom) != 0)
			{
				if (m_chunkbuf.empty())
				{
					if (!m_eof) m_errorhnd->report( _TXT( "unexpected end of input in '%s' segmenter"), SEGMENTER_NAME);
					return false;
				}
				else
				{
					m_chunkbuf.pop_front();
					if (!feedNextInputChunk()) return false;
					goto AGAIN; //... to set setjmp context again
				}
			}
			while (!m_xpathselect.getNext( id))
			{
				++m_itr;

				typename XMLScanner::ElementType et = m_itr->type();
				if (et == XMLScanner::ErrorOccurred)
				{
					const char* errstr = "";
					m_scanner.getError( &errstr);
					m_errorhnd->report( _TXT("error in document at position %u: %s"), (unsigned int)m_scanner.getTokenPosition(), errstr);
					return false;
				}
				else if (et == XMLScanner::Exit)
				{
					m_got_exit = true;
					return false;
				}
				m_xpathselect.putElement( m_itr->type(), m_itr->content(), m_itr->size());
			}
			pos = m_scanner.getTokenPosition();
			segment = m_itr->content();
			segmentsize = m_itr->size();
			return true;
		}
		CATCH_ERROR_MAP_ARG1_RETURN( _TXT("error in get next of '%s' segmenter: %s"), SEGMENTER_NAME, *m_errorhnd, false);
	}

private:
	typedef textwolf::XMLScanner<
			textwolf::SrcIterator,
			CharsetEncoding,
			textwolf::charset::UTF8,
			std::string
		> XMLScanner;

	const XPathAutomaton* m_automaton;
	XPathAutomatonContext m_xpathselect;
	textwolf::SrcIterator m_srciter;
	XMLScanner m_scanner;
	typename XMLScanner::iterator m_itr;
	typename XMLScanner::iterator m_end;
	char const* m_chunk;
	std::size_t m_chunksize;
	bool m_eof;
	bool m_got_exit;
	jmp_buf m_eom;
	std::list<std::string> m_chunkbuf;
	ErrorBufferInterface* m_errorhnd;
};

}//namespace
#endif


