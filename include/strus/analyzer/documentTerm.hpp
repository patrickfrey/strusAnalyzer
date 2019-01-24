/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Structure describing a typed document term
/// \file documentTerm.hpp
#ifndef _STRUS_ANALYZER_DOCUMENT_TERM_HPP_INCLUDED
#define _STRUS_ANALYZER_DOCUMENT_TERM_HPP_INCLUDED
#include <string>
#include <vector>

/// \brief strus toplevel namespace
namespace strus {
/// \brief analyzer parameter and return value objects namespace
namespace analyzer {

/// \brief Structure describing a typed document term
class DocumentTerm
{
public:
	typedef int Position;

	/// \brief Default constructor
	DocumentTerm()
		:m_type(),m_value(),m_pos(0){}
	/// \brief Copy constructor
#if __cplusplus >= 201103L
	DocumentTerm( DocumentTerm&& ) = default;
	DocumentTerm( const DocumentTerm& ) = default;
	DocumentTerm& operator= ( DocumentTerm&& ) = default;
	DocumentTerm& operator= ( const DocumentTerm& ) = default;
#else
	DocumentTerm( const DocumentTerm& o)
		:m_type(o.m_type),m_value(o.m_value),m_pos(o.m_pos){}
#endif
	/// \brief Constructor
	/// \param[in] t name of the term
	/// \param[in] v value of the term
	/// \param[in] p position of the term
	/// \param[in] l length of the term
	DocumentTerm( const std::string& t, const std::string& v, const Position& p)
		:m_type(t),m_value(v),m_pos(p){}

	/// \brief Get the type name of the term
	/// \return type name of the term
	const std::string& type() const		{return m_type;}
	/// \brief Get the value of the term
	/// \return value of the term
	const std::string& value() const	{return m_value;}
	/// \brief Get the ordinal position of the term
	/// \return ordinal position (word count index) of the term
	Position pos() const			{return m_pos;}

	/// \brief Set the ordinal position of the term
	/// \param[in] pos_ ordinal position (word count index) of the term
	void setPos( const Position& pos_)	{m_pos = pos_;}

	bool operator < (const DocumentTerm& o) const
	{
		if (m_pos == o.m_pos)
		{
			return (m_type == o.m_type)
				? m_value < o.m_value
				: m_type < o.m_type;
		}
		else
		{
			return m_pos < o.m_pos;
		}
	}

	bool defined() const
	{
		return !!m_pos;
	}

	void clear()
	{
		m_type.clear();
		m_value.clear();
		m_pos = 0;
	}

private:
	std::string m_type;
	std::string m_value;
	Position m_pos;
};

}}//namespace
#endif

