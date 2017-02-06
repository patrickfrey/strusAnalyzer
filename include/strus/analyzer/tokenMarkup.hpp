/*
 * Copyright (c) 2016 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Structure defining an annotation of text in one document
/// \file "tokenMarkup.hpp"
#ifndef _STRUS_ANALYZER_TOKEN_MARKUP_HPP_INCLUDED
#define _STRUS_ANALYZER_TOKEN_MARKUP_HPP_INCLUDED
#include <vector>
#include <string>

/// \brief strus toplevel namespace
namespace strus {
/// \brief analyzer parameter and return value objects namespace
namespace analyzer {

/// \brief Structure defining an annotation of text in a document
class TokenMarkup
{
public:
	/// \brief Structure describing a document markup attribute
	class Attribute
	{
	public:
		/// \brief Constructor
		Attribute( const std::string& name_, const std::string& value_)
			:m_name(name_),m_value(value_){}
		/// \brief Copy constructor
		Attribute( const Attribute& o)
			:m_name(o.m_name),m_value(o.m_value){}

		/// \brief Get the tag name of the markup attribute
		const std::string& name() const			{return m_name;}
		/// \brief Get the tag value of the markup attribute
		const std::string& value() const		{return m_value;}

	private:
		std::string m_name;
		std::string m_value;
	};

	/// \brief Default constructor
	TokenMarkup()
		:m_name(),m_attributes(){}
	/// \brief Constructor
	explicit TokenMarkup( const std::string& name_)
		:m_name(name_),m_attributes(){}
	/// \brief Constructor
	TokenMarkup( const std::string& name_, const std::vector<Attribute>& attributes_)
		:m_name(name_),m_attributes(attributes_){}
	/// \brief Copy constructor
	TokenMarkup( const TokenMarkup& o)
		:m_name(o.m_name),m_attributes(o.m_attributes){}

	/// \brief Get the tag name of the markup element
	const std::string& name() const				{return m_name;}
	/// \brief Get the list of attributes of the markup element
	const std::vector<Attribute>& attributes() const	{return m_attributes;}

	TokenMarkup& operator()( const std::string& name_, const std::string& value_)
	{
		m_attributes.push_back( Attribute( name_, value_));
		return *this;
	}

private:
	std::string m_name;
	std::vector<Attribute> m_attributes;
};

}}//namespace
#endif

