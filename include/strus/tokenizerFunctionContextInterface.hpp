/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Interface for the execution context of a tokenizer function
/// \file tokenizerFunctionContextInterface.hpp
#ifndef _STRUS_ANALYZER_TOKENIZER_FUNCTION_CONTEXT_INTERFACE_HPP_INCLUDED
#define _STRUS_ANALYZER_TOKENIZER_FUNCTION_CONTEXT_INTERFACE_HPP_INCLUDED
#include "strus/analyzer/token.hpp"
#include <utility>
#include <cstddef>
#include <vector>

#error DEPRECATED

/// \brief strus toplevel namespace
namespace strus {

/// \brief Interface to the context (state) for the execution of a tokenizer for one unit (document,query)
class TokenizerFunctionContextInterface
{
public:
	/// \brief Destructor
	virtual ~TokenizerFunctionContextInterface(){}

	/// \brief Tokenize a segment into a list of tokens
	/// \param[in] src pointer to segment to tokenize
	/// \param[in] srcsize size of the segment to tokenize in bytes
	virtual std::vector<analyzer::Token> tokenize( const char* src, std::size_t srcsize)=0;
};

}//namespace
#endif


