/*
 * Copyright (c) 2016 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Structure describing a token with id (an output item of char regex matching and an input item for token pattern matching)
/// \file "patternLexem.hpp"
#ifndef _STRUS_ANALYZER_PATTERN_LEXEM_HPP_INCLUDED
#define _STRUS_ANALYZER_PATTERN_LEXEM_HPP_INCLUDED
#include "strus/analyzer/token.hpp"
#include <cstddef>

namespace strus {
namespace analyzer {

/// \brief Structure describing a token with an id
/// \note Describes an output item of char regex matching and it is an input item for token pattern matching
class PatternLexem
	:public Token
{
public:
	/// \brief Default constructor
	PatternLexem()
		:m_id(0),Token(){}
	/// \brief Constructor
	PatternLexem( unsigned int id_, unsigned int ordpos_, std::size_t origseg_, std::size_t origpos_, std::size_t origsize_)
		:m_id(id_),Token(ordpos_,origseg_,origpos_,origsize_){}
	/// \brief Copy constructor
	PatternLexem( const PatternLexem& o)
		:m_id(o.m_id),Token(o){}
	/// \brief Destructor
	~PatternLexem(){}

	/// \brief Internal identifier of the term
	unsigned int id() const				{return m_id;}
	/// \brief Ordinal (counting) position assigned to the token
	unsigned int ordpos() const			{return ordpos();}
	/// \brief Original segment index of the token in the source
	std::size_t origseg() const			{return origseg();}
	/// \brief Original byte position of the token in the source segment as UTF-8 specified with origseg
	std::size_t origpos() const			{return origpos();}
	/// \brief Original byte size of the token in the source as UTF-8
	std::size_t origsize() const			{return origsize();}

private:
	unsigned int m_id;
};


}} //namespace
#endif

