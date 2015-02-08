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
#ifndef _STRUS_SEGMENTER_INTERFACE_HPP_INCLUDED
#define _STRUS_SEGMENTER_INTERFACE_HPP_INCLUDED
#include <utility>
#include <string>

namespace strus
{
/// \brief Forward declaration
class SegmenterInstanceInterface;

/// \brief Defines a program for splitting a source text it into chunks with an id correspoding to a selecting expression.
class SegmenterInterface
{
public:
	/// \brief Destructor
	virtual ~SegmenterInterface(){}

	/// \brief Defines an expression for selecting chunks from a document
	/// \param[in] id identifier of the chunks that match to expression
	/// \param[in] expression expression for selecting chunks
	virtual void defineSelectorExpression( int id, const std::string& expression)=0;

	/// \brief Creates an instance of the segmenter
	/// \param[in] source pointer to source. Expected to be valid the whole life time of the instance created
	/// \return the segmenter object to be desposed with delete by the caller
	virtual SegmenterInstanceInterface* createInstance( const char* source)=0;
};

}//namespace
#endif

