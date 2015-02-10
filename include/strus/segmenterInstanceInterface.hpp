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
#ifndef _STRUS_ANALYZER_SEGMENTER_INSTANCE_INTERFACE_HPP_INCLUDED
#define _STRUS_ANALYZER_SEGMENTER_INSTANCE_INTERFACE_HPP_INCLUDED
#include <utility>
#include <string>

namespace strus
{

/// \class SegmenterInstanceInterface
/// \brief Defines an instance of a segmenter program
class SegmenterInstanceInterface
{
public:
	/// \brief Destructor
	virtual ~SegmenterInstanceInterface(){}

	/// \brief Fetch the next chunk
	/// \param[out] id identifier of the expression that addresses the chunk
	/// \param[out] pos position of the chunk in the original source
	/// \param[out] chunk pointer to the start of the chunk. Must remain a valid reference during the whole lifetime of this segmented instance.
	/// \param[out] chunksize size of chunk in bytes
	/// \return true, if a valid chunk could be returned, false in case of EOF (no chunks left)
	/// \remark throws on error
	virtual bool getNext( int& id, std::size_t& pos, const char*& chunk, std::size_t& chunksize)=0;
};

}//namespace
#endif

