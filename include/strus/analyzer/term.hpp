/*
---------------------------------------------------------------------
    The C++ library strus implements basic operations to build
    a search engine for structured search on unstructured data.

    Copyright (C) 2015 Patrick Frey

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License as published by the Free Software Foundation; either
    version 3 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

--------------------------------------------------------------------

	The latest version of strus can be found at 'http://github.com/patrickfrey/strus'
	For documentation see 'http://patrickfrey.github.com/strus'

--------------------------------------------------------------------
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

private:
	std::string m_type;
	std::string m_value;
	unsigned int m_pos;
};

typedef std::vector<Term> TermVector;

}}//namespace
#endif

