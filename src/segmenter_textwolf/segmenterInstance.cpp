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

using namespace strus;

SegmenterInstance::SegmenterInstance( const Automaton* automaton_, std::istream& input_, std::size_t bufsize)
	:m_automaton(automaton_),m_scanner(textwolf::IStreamIterator(input_, bufsize)),m_pathselect(automaton_)
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

bool SegmenterInstance::getNext( int& id, SegmenterPosition& pos, const char*& chunk, std::size_t& chunksize)
{
	if (m_itr == m_end) return false;
	while (m_selitr == m_selend)
	{
		++m_itr;
		if (m_itr == m_end) return false;
		if (m_itr->type() == XMLScanner::ErrorOccurred)
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
		m_selitr = m_pathselect.push( m_itr->type(), m_itr->content(), m_itr->size());
		m_selend = m_pathselect.end();
	}
	id = *m_selitr;
	++m_selitr;
	pos = m_scanner.getIterator().position() - m_itr->size();
	chunk = m_itr->content();
	chunksize = m_itr->size();
	return true;
}


