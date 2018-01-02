/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Structure describing a typed query term
/// \file queryTerm.hpp
#ifndef _STRUS_ANALYZER_QUERY_TERM_HPP_INCLUDED
#define _STRUS_ANALYZER_QUERY_TERM_HPP_INCLUDED
#include <string>
#include <vector>

/// \brief strus toplevel namespace
namespace strus {
/// \brief analyzer parameter and return value objects namespace
namespace analyzer {

/// \brief Structure describing a typed query term
class QueryTerm
{
public:
	/// \brief Default constructor
	QueryTerm()
		:m_type(),m_value(),m_len(0){}
	/// \brief Copy constructor
	QueryTerm( const QueryTerm& o)
		:m_type(o.m_type),m_value(o.m_value),m_len(o.m_len){}
	/// \brief Constructor
	/// \param[in] t name of the term
	/// \param[in] v value of the term
	/// \param[in] l length of the term
	QueryTerm( const std::string& t, const std::string& v, int l)
		:m_type(t),m_value(v),m_len(l){}

	/// \brief Get the type name of the term
	/// \return type name of the term
	const std::string& type() const		{return m_type;}
	/// \brief Get the value of the term
	/// \return value of the term
	const std::string& value() const	{return m_value;}
	/// \brief Get the length of the term (ordinal position count)
	/// \return ordinal position count of the term
	int len() const				{return m_len;}

	/// \brief Set the length of the term (ordinal position count)
	/// \param[in] len_ length (term count) of the term
	void setLen( int len_)			{m_len = len_;}

private:
	std::string m_type;
	std::string m_value;
	int m_len;
};

}}//namespace
#endif

