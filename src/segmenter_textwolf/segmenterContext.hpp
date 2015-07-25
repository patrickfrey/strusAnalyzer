/*
---------------------------------------------------------------------
    The C++ library strus implements basic operations to build
    a search engine for structured search on unstructured data.

    Copyright (C) 2013,2014 Patrick Frey

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

--------------------------------------------------------------------

	The latest version of strus can be found at 'http://github.com/patrickfrey/strus'
	For documentation see 'http://patrickfrey.github.com/strus'

--------------------------------------------------------------------
*/
#ifndef _STRUS_SEGMENTER_CONTEXT_TEXTWOLF_HPP_INCLUDED
#define _STRUS_SEGMENTER_CONTEXT_TEXTWOLF_HPP_INCLUDED
#include "strus/segmenterContextInterface.hpp"
#include "segmenter.hpp"
#include "textwolf/xmlpathselect.hpp"
#include "textwolf/charset.hpp"
#include "textwolf/sourceiterator.hpp"
#include <stdint.h>
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
	explicit SegmenterContext( const Automaton* automaton_, const CharsetEncoding& charset_=CharsetEncoding())
		:m_automaton(automaton_)
		,m_scanner(charset_,textwolf::SrcIterator())
		,m_pathselect(automaton_)
		,m_chunk(0),m_chunksize(0),m_eof(false),m_done(false)
	{
		m_selitr = m_selend = m_pathselect.end();
	}

	virtual void putInput( const char* chunk, std::size_t chunksize, bool eof)
	{
		if (m_eof)
		{
			throw std::runtime_error("feeded chunk after declared end of input");
		}
		if (m_chunk != 0 || !m_scanner.getIterator().endOfChunk())
		{
			throw std::runtime_error("feeded chunk to segmenter while previous chunk is still processed");
		}
		m_chunk = chunk;
		m_chunksize = chunksize;
		m_eof = eof;
	}

	virtual bool getNext( int& id, SegmenterPosition& pos, const char*& segment, std::size_t& segmentsize)
	{
		jmp_buf eom;
		if (setjmp(eom) != 0)
		{
			if (m_eof)
			{
				if (!m_done)
				{
					m_chunk = "";
					m_chunksize = 1;
					m_done = true;
				}
				else
				{
					throw std::runtime_error( "unexpected end of input");
				}
			}
			else
			{
				return false; //... do call the function with the next chunk or exit (eof)
			}
		}
		if (m_chunk)
		{
			m_srciter.putInput( m_chunk, m_chunksize, &eom);
			m_scanner.setSource( m_srciter);
			m_chunk = 0;
			m_chunksize = 0;
			m_itr = m_scanner.begin(false);
			m_end = m_scanner.end();
		}
		else
		{
			m_srciter.setErrorExit( &eom);
			m_scanner.setSource( m_srciter);
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
				throw std::runtime_error( std::string( "error in XML document: ") + errstr);
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
	const char* m_chunk;
	std::size_t m_chunksize;
	bool m_eof;
	bool m_done;
};

}//namespace
#endif


