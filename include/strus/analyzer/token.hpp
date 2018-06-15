/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Structure describing a token in the document by its start position and size
/// \file token.hpp
#ifndef _STRUS_ANALYZER_TOKENIZER_TOKEN_HPP_INCLUDED
#define _STRUS_ANALYZER_TOKENIZER_TOKEN_HPP_INCLUDED
#include "strus/analyzer/position.hpp"
#include <string>

/// \brief strus toplevel namespace
namespace strus {
/// \brief analyzer parameter and return value objects namespace
namespace analyzer {

/// \brief Structure describing a token in the document by its start and end position
struct Token
{
public:
	/// \brief Default constructor
	Token()
		:m_ordpos(0),m_origsize(0),m_origpos(){}
	/// \brief Constructor
	/// \param[in] ordpos_ oridinal term position assigned to the the token
	/// \param[in] origpos_ position of the token start in the original document
	/// \param[in] origsize_ size of the token in bytes in the translated document segment (UTF-8)
	Token( int ordpos_, const Position& origpos_, int origsize_)
		:m_ordpos(ordpos_),m_origsize(origsize_),m_origpos(origpos_){}
	/// \brief Copy constructor
	Token( const Token& o)
		:m_ordpos(o.m_ordpos),m_origsize(o.m_origsize),m_origpos(o.m_origpos){}

	///\brief Get the ordinal (counting) position in the document. This value is used to assign the term position, that is not the byte position but a number taken from the enumeration of all distinct feature byte postions
	int ordpos() const		{return m_ordpos;}
	///\brief Get the start position of this token in the original document
	const Position& origpos() const	{return m_origpos;}
	///\brief Get the start position of this token in the original document
	Position& origpos()		{return m_origpos;}
	///\brief Get the byte size of the translated token string (UTF-8) in the original document segment
	int origsize() const		{return m_origsize;}

	/// \brief Set the original segment index of the token in the source
	/// \param[in] origpos_ position of the token start in the original document
	void setOrigPosition( const Position& origpos_)
	{
		m_origpos = origpos_;
	}

	/// \brief Set the ordinal position of the token in the source (adjusted in case of multiple segments)
	/// \param[in] ordpos_ oridinal term position assigned to the the token
	void setOrdpos( int ordpos_)
	{
		m_ordpos = ordpos_;
	}

	/// \brief Compare with another token
	int compare( const Token& o) const
	{
		int rt = m_origpos.compare( o.m_origpos);
		if (rt) return rt;
		return ((m_origsize < o.m_origsize) - (m_origsize > o.m_origsize));
	}
	/// \brief Compare operator for sort
	bool operator < (const Token& o) const
	{
		int cmp = m_origpos.compare( o.m_origpos);
		return (cmp == 0) ? (m_origsize < o.m_origsize) : cmp < 0;
	}

private:
	uint32_t m_ordpos;	///< ordinal (counting) position in the document. This value is used to assign the term position, that is not the byte position but a number taken from the enumeration of all distinct feature byte postions
	uint32_t m_origsize;	///< byte size of the translated token string (UTF-8) in the original document segment
	Position m_origpos;	///< original position
};

}}//namespace
#endif

