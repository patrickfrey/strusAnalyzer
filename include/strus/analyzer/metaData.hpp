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
/// \brief Structure describing a document meta data element
/// \file metaData.hpp
#ifndef _STRUS_ANALYZER_METADATA_HPP_INCLUDED
#define _STRUS_ANALYZER_METADATA_HPP_INCLUDED

/// \brief strus toplevel namespace
namespace strus {
/// \brief analyzer parameter and return value objects namespace
namespace analyzer {

/// \brief Structure describing a document meta data element
class MetaData
{
public:
	/// \brief Default constructor
	MetaData(){}
	/// \brief Copy constructor
	MetaData( const MetaData& o)
		:m_name(o.m_name),m_value(o.m_value){}
	/// \brief Constructor
	/// \param[in] n name of the meta data element
	/// \param[in] v value of the meta data element
	MetaData( const std::string& n, const std::string& v)
		:m_name(n),m_value(v){}

	/// \brief Get the name of the meta data element
	/// \return Name of the meta data element
	const std::string& name() const		{return m_name;}
	/// \brief Get the value of the meta data element
	/// \return Value of the meta data element
	const std::string& value() const	{return m_value;}

private:
	std::string m_name;
	std::string m_value;
};

}}//namespace
#endif

