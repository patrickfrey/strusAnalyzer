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
/// \brief Structure describing a document attribute
/// \file attribute.hpp
#ifndef _STRUS_ANALYZER_ATTRIBUTE_HPP_INCLUDED
#define _STRUS_ANALYZER_ATTRIBUTE_HPP_INCLUDED
#include <string>

/// \brief strus toplevel namespace
namespace strus {
/// \brief analyzer parameter and return value objects namespace
namespace analyzer {

/// \brief Structure describing a document attribute
class Attribute
{
public:
	/// \brief Default constructor
	Attribute(){}
	/// \brief Copy constructor
	Attribute( const Attribute& o)
		:m_name(o.m_name),m_value(o.m_value){}
	/// \brief Constructor
	/// \param[in] n name of the attribute
	/// \param[in] v value of the attribute
	Attribute( const std::string& n, const std::string& v)
		:m_name(n),m_value(v){}

	/// \brief Get the name of the attribute
	/// \return Name of the attribute
	const std::string& name() const		{return m_name;}
	/// \brief Get the value of the attribute
	/// \return Value of the attribute
	const std::string& value() const	{return m_value;}

private:
	std::string m_name;
	std::string m_value;
};

}}//namespace
#endif

