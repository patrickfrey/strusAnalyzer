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
#include "private/errorUtils.hpp"
#include "strus/errorBufferInterface.hpp"
#include "segmenter.hpp"
#include "private/xpathAutomaton.hpp"
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
		,m_chunkbufsize(0)
		,m_eof(false)
		,m_got_eom(true)
		,m_chunkbufitr(0)
		,m_chunkbufeof(false)
		,m_errorhnd(errorhnd)
	{
	}

	virtual ~SegmenterContext()
	{
		if (m_chunk) std::free( m_chunk);
	}

	virtual void putInput( const char* chunk, std::size_t chunksize, bool eof)
	{
		try
		{
			if (m_eof)
			{
				m_errorhnd->report( "feeded chunk after declared end of input");
				return;
			}
			if (!m_got_eom)
			{
				try
				{
					m_chunkbuf.push_back( std::string( chunk, chunksize));
					m_chunkbufeof = eof;
				}
				catch (const std::bad_alloc&)
				{
					m_errorhnd->report( "out of memory when buffering chunks for 'textwolf' segmenter");
				}
				return;
			}
			std::size_t chunkallocsize = chunksize + 4;
			if (chunkallocsize > m_chunkbufsize)
			{
				void* chunkmem = std::realloc( m_chunk, chunkallocsize);
				if (!chunkmem)
				{
					m_errorhnd->report( "out of memory in 'textwolf' segmenter");
					return;
				}
				m_chunk = (char*)chunkmem;
				m_chunkbufsize = chunkallocsize;
			}
			m_eof = eof;
			if (eof)
			{
				m_chunksize = chunkallocsize;
				std::memcpy( m_chunk, chunk, chunksize);
				std::memset( m_chunk + chunksize, 0, 4);
			}
			else
			{
				m_chunksize = chunksize;
				std::memcpy( m_chunk, chunk, chunksize);
			}
			m_srciter.putInput( m_chunk, m_chunksize, &m_eom);
			m_scanner.setSource( m_srciter);
			m_itr = m_scanner.begin(false);
			m_end = m_scanner.end();
			m_got_eom = (m_chunksize == 0);
		}
		CATCH_ERROR_MAP_ARG1( _TXT("error in put input of '%s' segmenter: %s"), SEGMENTER_NAME, *m_errorhnd);
	}

	virtual bool getNext( int& id, SegmenterPosition& pos, const char*& segment, std::size_t& segmentsize)
	{
		try
		{
		AGAIN:
			if (setjmp(m_eom) != 0)
			{
				if (m_eof)
				{
					m_errorhnd->report( "unexpected end of input");
					return false;
				}
				else if (m_chunkbufitr < m_chunkbuf.size())
				{
					m_got_eom = true;
					bool ceof = m_chunkbufeof && (m_chunkbufitr+1) == m_chunkbuf.size();
					putInput( m_chunkbuf[ m_chunkbufitr].c_str(), m_chunkbuf[ m_chunkbufitr].size(), ceof);
					++m_chunkbufitr;
					goto AGAIN;
				}
				else
				{
					m_got_eom = true;
					return false; //... do call the function with the next chunk or exit (eof)
				}
			}
			if (m_itr == m_end) return false;
			while (!m_xpathselect.getNext( id))
			{
				++m_itr;
				if (m_itr == m_end) return false;
	
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
	char* m_chunk;
	std::size_t m_chunksize;
	std::size_t m_chunkbufsize;
	bool m_eof;
	bool m_got_eom;
	jmp_buf m_eom;
	std::vector<std::string> m_chunkbuf;
	std::size_t m_chunkbufitr;
	bool m_chunkbufeof;
	ErrorBufferInterface* m_errorhnd;
};

}//namespace
#endif


