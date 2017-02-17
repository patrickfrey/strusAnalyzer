/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Structure describing a token in the document by its start and end position
/// \file token.hpp
#ifndef _STRUS_ANALYZER_TOKENIZER_TOKEN_HPP_INCLUDED
#define _STRUS_ANALYZER_TOKENIZER_TOKEN_HPP_INCLUDED
#include "strus/base/stdint.h"
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
		:m_ordpos(0),m_origseg(0),m_origpos(0),m_origsize(0){}
	/// \brief Constructor
	/// \param[in] ordpos_ oridinal term position assigned to the the token
	/// \param[in] origseg_ start byte position of the document segment
	/// \param[in] origpos_ byte position of the token start in the translated document segment (UTF-8)
	/// \param[in] origsize_ size of the token in bytes in the translated document segment (UTF-8)
	Token( unsigned int ordpos_, unsigned int origseg_, unsigned int origpos_, unsigned int origsize_)
		:m_ordpos(ordpos_),m_origseg(origseg_),m_origpos(origpos_),m_origsize(origsize_){}
	/// \brief Copy constructor
	Token( const Token& o)
		:m_ordpos(o.m_ordpos),m_origseg(o.m_origseg),m_origpos(o.m_origpos),m_origsize(o.m_origsize){}

	///\brief Get the ordinal (counting) position in the document. This value is used to assign the term position, that is not the byte position but a number taken from the enumeration of all distinct feature byte postions
	unsigned int ordpos() const	{return m_ordpos;}
	///\brief Get the start byte position of the document segment of this token
	unsigned int origseg() const	{return m_origseg;}
	///\brief Get the start byte position of the token string in the translated document segment (UTF-8)
	unsigned int origpos() const	{return m_origpos;}
	///\brief Get the byte size of the translated token string (UTF-8) in the original document segment
	unsigned int origsize() const	{return m_origsize;}

	/// \brief Set the original segment index of the token in the source
	void setOrigseg( std::size_t origseg_)
	{
		m_origseg = origseg_;
	}

	bool operator < (const Token& o) const
	{
		return (m_origseg == o.m_origseg)
			? (
				(m_origpos == o.m_origpos)
				? (m_origsize < o.m_origsize)
				: (m_origpos < o.m_origpos)
			)
			: (m_origseg < o.m_origseg);
	}

private:
	uint32_t m_ordpos;	///< ordinal (counting) position in the document. This value is used to assign the term position, that is not the byte position but a number taken from the enumeration of all distinct feature byte postions
	uint32_t m_origseg;	///< start byte position of the document segment
	uint32_t m_origpos;	///< start byte position of the token string in the translated document segment (UTF-8)
	uint32_t m_origsize;	///< byte size of the translated token string (UTF-8) in the original document segment
};

}}//namespace
#endif

