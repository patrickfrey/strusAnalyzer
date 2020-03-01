/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Exported functions of the strus analyzer word tokenization library
/// \file tokenizer_word.hpp
#ifndef _STRUS_ANALYZER_TOKENIZER_WORD_LIB_HPP_INCLUDED
#define _STRUS_ANALYZER_TOKENIZER_WORD_LIB_HPP_INCLUDED

/// \brief strus toplevel namespace
namespace strus
{

/// \brief Forward declaration
class TokenizerFunctionInterface;
/// \brief Forward declaration
class ErrorBufferInterface;

/// \brief Get the tokenizer type that creates the tokenization of fields in a query
/// \return the tokenization function
TokenizerFunctionInterface* createTokenizer_queryfield( ErrorBufferInterface* errorhnd);
/// \brief Get the tokenizer type that creates the tokenization of words in the input
/// \return the tokenization function
TokenizerFunctionInterface* createTokenizer_word( ErrorBufferInterface* errorhnd);
/// \brief Get the tokenizer type that creates the tokenization as splitting of the input by whitespaces 
/// \return the tokenization function
TokenizerFunctionInterface* createTokenizer_whitespace( ErrorBufferInterface* errorhnd);
/// \brief Get the tokenizer type that creates the tokenization as splitting of all tokens, returning sequences of language characters as tokens and word boundary delimiters as single character
/// \return the tokenization function
TokenizerFunctionInterface* createTokenizer_langtoken( ErrorBufferInterface* errorhnd);
/// \brief Get the tokenizer type that creates the tokenization as words containing sequences of language alphabet characters
/// \return the tokenization function
TokenizerFunctionInterface* createTokenizer_alpha( ErrorBufferInterface* errorhnd);

}//namespace
#endif

