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
#include "strus/errorBufferInterface.hpp"
#include "segmenter.hpp"
#include "textwolf/xmlpathselect.hpp"
#include "textwolf/charset.hpp"
#include "textwolf/sourceiterator.hpp"
#include <cstdlib>
#include <setjmp.h>

namespace strus
{

template <class CharsetEncoding>
class SegmenterContext
	:public SegmenterContextInterface
{
public:
	typedef textwolf::XMLPathSelectAutomaton<> Automaton;

public:
	explicit SegmenterContext( ErrorBufferInterface* errorhnd, const Automaton* automaton_, const CharsetEncoding& charset_=CharsetEncoding())
		:m_automaton(automaton_)
		,m_scanner(charset_,textwolf::SrcIterator())
		,m_pathselect(automaton_)
		,m_chunk(0)
		,m_chunksize(0)
		,m_chunkbufsize(0)
		,m_eof(false)
		,m_got_eom(true)
		,m_errorhnd(errorhnd)
	{
		m_selitr = m_selend = m_pathselect.end();
	}

	virtual ~SegmenterContext()
	{
		if (m_chunk) std::free( m_chunk);
	}

	virtual void putInput( const char* chunk, std::size_t chunksize, bool eof)
	{
		if (m_eof)
		{
			m_errorhnd->report( "feeded chunk after declared end of input");
			return;
		}
		if (!m_got_eom)
		{
			m_errorhnd->report( "feeded chunk to segmenter while previous chunk is still processed");
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

	virtual bool getNext( int& id, SegmenterPosition& pos, const char*& segment, std::size_t& segmentsize)
	{
		if (setjmp(m_eom) != 0)
		{
			if (m_eof)
			{
				m_errorhnd->report( "unexpected end of input");
				return false;
			}
			else
			{
				m_got_eom = true;
				return false; //... do call the function with the next chunk or exit (eof)
			}
		}
		if (m_itr == m_end) return false;
		while (m_selitr == m_selend)
		{
			++m_itr;
			if (m_itr == m_end) return false;
	
			typename XMLScanner::ElementType et = m_itr->type();
			if (et == XMLScanner::ErrorOccurred)
			{
				const char* errstr = "";
				m_scanner.getError( &errstr);
				m_errorhnd->report( "error in XML document: %s", errstr);
				return false;
			}
			else if (et == XMLScanner::Exit)
			{
				return false;
			}
			m_selitr = m_pathselect.push( m_itr->type(), m_itr->content(), m_itr->size());
			m_selend = m_pathselect.end();
		}
		id = *m_selitr;
		++m_selitr;
		pos = m_scanner.getIterator().position() - m_itr->size();
		segment = m_itr->content();
		segmentsize = m_itr->size();
		return true;
	}

private:
	typedef textwolf::XMLPathSelect<
			textwolf::charset::UTF8
		> XMLPathSelect;
	typedef textwolf::XMLScanner<
			textwolf::SrcIterator,
			CharsetEncoding,
			textwolf::charset::UTF8,
			std::string
		> XMLScanner;

	const Automaton* m_automaton;
	textwolf::SrcIterator m_srciter;
	XMLScanner m_scanner;
	XMLPathSelect m_pathselect;
	typename XMLScanner::iterator m_itr;
	typename XMLScanner::iterator m_end;
	XMLPathSelect::iterator m_selitr;
	XMLPathSelect::iterator m_selend;
	char* m_chunk;
	std::size_t m_chunksize;
	std::size_t m_chunkbufsize;
	bool m_eof;
	bool m_got_eom;
	jmp_buf m_eom;
	ErrorBufferInterface* m_errorhnd;
};

}//namespace
#endif


