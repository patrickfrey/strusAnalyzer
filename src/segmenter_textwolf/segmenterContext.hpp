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
#include "textwolf/xmlpathselect.hpp"
#include "textwolf/charset.hpp"
#include "textwolf/sourceiterator.hpp"
#include <stdint.h>

namespace strus
{

class SegmenterContext
	:public SegmenterContextInterface
{
public:
	typedef textwolf::XMLPathSelectAutomaton<> Automaton;

public:
	explicit SegmenterContext( const Automaton* automaton_);

	virtual void putInput( const char* chunk, std::size_t chunksize, bool eof);

	virtual bool getNext( int& id, SegmenterPosition& pos, const char*& segment, std::size_t& segmentsize);

private:
	typedef textwolf::XMLPathSelect<
			textwolf::charset::UTF8
		> XMLPathSelect;
	typedef textwolf::XMLScanner<
			textwolf::SrcIterator,
			textwolf::charset::UTF8,
			textwolf::charset::UTF8,
			std::string
		> XMLScanner;

	const Automaton* m_automaton;
	textwolf::SrcIterator m_srciter;
	XMLScanner m_scanner;
	XMLPathSelect m_pathselect;
	XMLScanner::iterator m_itr;
	XMLScanner::iterator m_end;
	XMLPathSelect::iterator m_selitr;
	XMLPathSelect::iterator m_selend;
	const char* m_chunk;
	std::size_t m_chunksize;
	bool m_eof;
	bool m_done;
};

}//namespace
#endif

