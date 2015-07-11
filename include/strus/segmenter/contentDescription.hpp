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
/// \brief Structure describing the content type and format of an original document
/// \file contentDescription.hpp
#ifndef _STRUS_ANALYZER_CONTENT_DESCRIPTION_HPP_INCLUDED
#define _STRUS_ANALYZER_CONTENT_DESCRIPTION_HPP_INCLUDED
#include <vector>
#include <string>

/// \brief strus toplevel namespace
namespace strus {
/// \brief analyzer parameter and return value objects namespace
namespace segmenter {

/// \brief Defines a description of the properties of an original document processed by the segmenter
class ContentDescription
{
public:
	/// \brief Properties of a content
	enum Property
	{
		ContentType,		///< content MIME type
		Encoding,		///< character set encoding ("UTF-8", "UTF-16BE", etc...)
		ColumnName		///< name of a column, for content with an external table description
	};

	/// \brief Default constructor
	ContentDescription(){}
	/// \brief Copy constructor
	ContentDescription( const ContentDescription& o)
		:m_descr(o.m_descr){}

	/// \brief Set a property of the associated content
	/// \param[in] p property identifier
	/// \param[in] v value of property
	void setProperty( const Property& p, const std::string& value)
	{
		m_descr.push_back( Element( p, value));
	}

	/// \brief Get a property of the associated content
	/// \param[in] p property identifier
	/// \return value of property or NULL, if it does not exist
	const char* getProperty( const Property& p) const
	{
		std::vector<Element>::const_iterator pi = m_descr.begin(), pe = m_descr.end();
		for (; pi != pe; ++pi)
		{
			if (p == pi->first) return pi->second.c_str();
		}
		return 0;
	}

	/// \brief Get a multi value property of the associated content
	/// \param[in] p property identifier
	/// \return list of properties or empty, if it does not exist
	std::vector<std::string> getProperties( const Property& p) const
	{
		std::vector<std::string> rt;
		std::vector<Element>::const_iterator pi = m_descr.begin(), pe = m_descr.end();
		for (; pi != pe; ++pi)
		{
			if (p == pi->first) rt.push_back( pi->second);
		}
		return rt;
	}

private:
	typedef std::pair<Property,std::string> Element;
	std::vector<Element> m_descr;
};

}}//namespace
#endif

