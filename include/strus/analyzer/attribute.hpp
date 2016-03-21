/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
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

