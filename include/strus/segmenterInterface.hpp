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
/// \brief Interface for the document segmenter
/// \file segmenterInterface.hpp
#ifndef _STRUS_ANALYZER_SEGMENTER_INTERFACE_HPP_INCLUDED
#define _STRUS_ANALYZER_SEGMENTER_INTERFACE_HPP_INCLUDED

/// \brief strus toplevel namespace
namespace strus
{
/// \brief Forward declaration
class SegmenterInstanceInterface;
/// \brief Forward declaration
class AnalyzerErrorBufferInterface;

/// \class SegmenterInterface
/// \brief Defines an interface for creating instances of programs for document segmentation
class SegmenterInterface
{
public:
	/// \brief Destructor
	virtual ~SegmenterInterface(){}

	/// \brief Get the mime type accepted by this segmenter
	/// \return the mime type string
	virtual const char* mimeType() const=0;

	/// \brief Create a parameterizable segmenter instance
	/// \param[in] errorhnd analyzer error buffer interface for reporting exeptions and errors
	virtual SegmenterInstanceInterface* createInstance( AnalyzerErrorBufferInterface* errorhnd) const=0;
};

}//namespace
#endif


