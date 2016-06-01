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
#include <string>

/// \brief strus toplevel namespace
namespace strus {
/// \brief analyzer parameter and return value objects namespace
namespace analyzer {

/// \brief Structure describing a token in the document by its start and end position
struct Token
{
	unsigned int ordpos;	///< ordinal (counting) position in the document. This value is used to assign the term position, that is not the byte position but a number taken from the enumeration of all distinct feature byte postions
	unsigned int strpos;	///< start byte position of the token string in the original document segment
	unsigned int strsize;	///< byte size of the token string in the original document segment

	/// \brief Constructor
	/// \param[in] ordpos_ word position of the token starred
	/// \param[in] strpos_ byte position of the token start
	/// \param[in] strsize_ size of the token in bytes
	Token( unsigned int ordpos_, unsigned int strpos_, unsigned int strsize_)
		:ordpos(ordpos_),strpos(strpos_),strsize(strsize_){}
	/// \brief Copy constructor
	Token( const Token& o)
		:ordpos(o.ordpos),strpos(o.strpos),strsize(o.strsize){}
};

}}//namespace
#endif

