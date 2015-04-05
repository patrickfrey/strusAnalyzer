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
#include "segmenterInstance.hpp"
#include "segmenter.hpp"
#include <setjmp.h>

using namespace strus;

SegmenterInstance::SegmenterInstance( const Automaton* automaton_)
	:m_automaton(automaton_)
	,m_scanner(textwolf::SrcIterator())
	,m_pathselect(automaton_)
	,m_chunk(0),m_chunksize(0),m_eof(false)
{
	m_itr = m_scanner.begin();
	m_end = m_scanner.end();
	if (m_itr == m_end)
	{
		m_selitr = m_selend = m_pathselect.end();
	}
	else
	{
		m_selitr = m_pathselect.push( m_itr->type(), m_itr->content(), m_itr->size());
		m_selend = m_pathselect.end();
	}
}

void SegmenterInstance::putInput( const char* chunk, std::size_t chunksize, bool eof)
{
	if (m_eof)
	{
		throw std::runtime_error("feeded chunk after declared end of input");
	}
	if (m_chunk != 0 || m_scanner.getIterator().endOfChunk())
	{
		throw std::runtime_error("feeded chunk to segmenter while previous chunk is still processed");
	}
	m_chunk = chunk;
	m_chunksize = chunksize;
	m_eof = eof;
}

bool SegmenterInstance::getNext( int& id, SegmenterPosition& pos, const char*& segment, std::size_t& segmentsize)
{
	jmp_buf eom;
	if (m_chunk)
	{
		m_scanner.getIterator().putInput( m_chunk, m_chunksize, &eom);
		m_chunk = 0;
		m_chunksize = 0;
	}
	if (setjmp(eom) != 0)
	{
		if (m_eof)
		{
			throw std::runtime_error( "unexpected end of input");
		}
		return false; //... do call the function with the next chunk
	}
	if (m_itr == m_end) return false;
	while (m_selitr == m_selend)
	{
		++m_itr;
		XMLScanner::ElementType et = m_itr->type();
		if (et == XMLScanner::ErrorOccurred)
		{
			if (m_itr->size())
			{
				throw std::runtime_error( std::string( "error in XML document: ") + std::string(m_itr->content(), m_itr->size()));
			}
			else
			{
				throw std::runtime_error( std::string( "input document is not valid XML"));
			}
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


