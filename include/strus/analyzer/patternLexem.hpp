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

/// \brief Structure describing a token with an id used for pattern matching
class PatternLexem
	:public Token
{
public:
	/// \brief Default constructor
	PatternLexem()
		:Token(),m_id(0){}
	/// \brief Constructor
	PatternLexem( unsigned int id_, unsigned int ordpos_, std::size_t origseg_, std::size_t origpos_, std::size_t origsize_)
		:Token(ordpos_,origseg_,origpos_,origsize_),m_id(id_){}
	/// \brief Copy constructor
	PatternLexem( const PatternLexem& o)
		:Token(o),m_id(o.m_id){}
	/// \brief Destructor
	~PatternLexem(){}

	/// \brief Internal identifier of the term
	unsigned int id() const				{return m_id;}

private:
	unsigned int m_id;
};


}} //namespace
#endif

