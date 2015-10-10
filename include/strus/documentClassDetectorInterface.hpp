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
/// \brief Interface for detecting the document class of a content
/// \file documentClassDetectorInterface.hpp
#ifndef _STRUS_ANALYZER_DOCUMENT_CLASS_DETECTOR_INTERFACE_HPP_INCLUDED
#define _STRUS_ANALYZER_DOCUMENT_CLASS_DETECTOR_INTERFACE_HPP_INCLUDED
#include "strus/documentClass.hpp"
#include <vector>
#include <string>

/// \brief strus toplevel namespace
namespace strus
{

/// \brief Defines a detector that returns a content description for a document content it recognizes
class DocumentClassDetectorInterface
{
public:
	/// \brief Destructor
	virtual ~DocumentClassDetectorInterface(){}

	/// \brief Scans the start of a document to detect its classification attributes (mime type, etc.)
	/// \param[in,out] dclass document class to edit
	/// \param[in] contentBegin start of content begin chunk
	/// \param[in] contentBeginSize size of content begin chunk
	/// \param[in] errorhnd analyzer error buffer interface for reporting exeptions and errors
	/// \return true, if the document class was recognized
	/// \note It is assumed that a reasonable size of the document chunk (e.g. 1K) is enough to detect the document class. This is an assumption that is wrong for many MIME types, but it should work for text content. At least it should be enough to recognize the segmenter to use.
	virtual bool detect( DocumentClass& dclass, const char* contentBegin, std::size_t contentBeginSize) const=0;
};

}//namespace
#endif

