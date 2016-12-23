/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Interface for a parameterized tokenizer function
/// \file tokenizerFunctionInstanceInterface.hpp
#ifndef _STRUS_ANALYZER_TOKENIZER_FUNCTION_INSTANCE_INTERFACE_HPP_INCLUDED
#define _STRUS_ANALYZER_TOKENIZER_FUNCTION_INSTANCE_INTERFACE_HPP_INCLUDED
#include "strus/analyzer/token.hpp"
#include <string>
#include <vector>

/// \brief strus toplevel namespace
namespace strus {

/// \brief Interface for tokenization
class TokenizerFunctionInstanceInterface
{
public:
	/// \brief Destructor
	virtual ~TokenizerFunctionInstanceInterface(){}

	/// \brief Flag defined by tokenizer indicating that different segments defined by the tag hierarchy should be concatenated before tokenization
	/// \return true, if the argument chunks should be passed as one concatenated string, else if no
	/// \remark This flag is needed for context sensitive tokenization like for example for recognizing punctuation.
	virtual bool concatBeforeTokenize() const=0;

	/// \brief Tokenize a segment into a list of tokens
	/// \param[in] src pointer to segment to tokenize
	/// \param[in] srcsize size of the segment to tokenize in bytes
	virtual std::vector<analyzer::Token> tokenize( const char* src, std::size_t srcsize) const=0;
};

}//namespace
#endif


