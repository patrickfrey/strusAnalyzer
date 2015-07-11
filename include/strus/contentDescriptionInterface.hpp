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
/// \brief Interface to describe the content type and format of an original document
/// \file contentDescriptionInterface.hpp
#ifndef _STRUS_ANALYZER_CONTENT_DESCRIPTION_INTERFACE_HPP_INCLUDED
#define _STRUS_ANALYZER_CONTENT_DESCRIPTION_INTERFACE_HPP_INCLUDED
#include <vector>
#include <string>

/// \brief strus toplevel namespace
namespace strus
{

/// \brief Defines a description of the properties of an original document processed by the analyzer
class ContentDescriptionInterface
{
public:
	/// \brief Destructor
	virtual ~ContentDescriptionInterface(){}

	/// \brief Properties of a content
	enum Property
	{
		ContentType,		///< content MIME type
		Encoding,		///< character set encoding ("UTF-8", "UTF-16BE", etc...)
		ColumnName		///< name of a column, for content with an external table description
	};

	/// \brief Get a property of the content
	/// \param[in] p property identifier
	/// \param[in] i index of property
	/// \return value of property or NULL, if it does not exist
	virtual const char* getProperty( const Property& p, int i=0) const=0;
};

}//namespace
#endif

