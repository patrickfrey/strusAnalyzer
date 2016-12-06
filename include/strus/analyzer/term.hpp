/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Structure describing a typed document term
/// \file term.hpp
#ifndef _STRUS_ANALYZER_TERM_HPP_INCLUDED
#define _STRUS_ANALYZER_TERM_HPP_INCLUDED
#include <string>
#include <vector>

/// \brief strus toplevel namespace
namespace strus {
/// \brief analyzer parameter and return value objects namespace
namespace analyzer {

/// \brief Structure describing a typed document term
class Term
{
public:
	/// \brief Default constructor
	Term()
		:m_pos(0){}
	/// \brief Copy constructor
	Term( const Term& o)
		:m_type(o.m_type),m_value(o.m_value),m_pos(o.m_pos){}
	/// \brief Constructor
	/// \param[in] t name of the term
	/// \param[in] v value of the term
	/// \param[in] p position of the term
	Term( const std::string& t, const std::string& v, unsigned int p)
		:m_type(t),m_value(v),m_pos(p){}

	/// \brief Get the type name of the term
	/// \return Type name of the term
	const std::string& type() const		{return m_type;}
	/// \brief Get the value of the term
	/// \return Value of the term
	const std::string& value() const	{return m_value;}
	/// \brief Get the position of the term
	/// \return Word position of the term in the document
	unsigned int pos() const		{return m_pos;}

	/// \brief Set the word position of the term
	/// \param[in] pos_ position (word count index) of the term in the document
	void setPos( unsigned int pos_)		{m_pos = pos_;}

	bool operator < (const Term& o) const
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

private:
	std::string m_type;
	std::string m_value;
	unsigned int m_pos;
};

typedef std::vector<Term> TermArray;

}}//namespace
#endif

