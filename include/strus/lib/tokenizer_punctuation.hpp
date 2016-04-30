/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Exported functions of the strus analyzer punctuation tokenization library
/// \file tokenizer_punctuation.hpp
#ifndef _STRUS_ANALYZER_TOKENIZER_PUNCTUATION_LIB_HPP_INCLUDED
#define _STRUS_ANALYZER_TOKENIZER_PUNCTUATION_LIB_HPP_INCLUDED

/// \brief strus toplevel namespace
namespace strus
{

/// \brief Forward declaration
class TokenizerFunctionInterface;
/// \brief Forward declaration
class ErrorBufferInterface;

/// \brief Get the tokenizer type that creates the tokenization of punctuation elements in the input
/// \return the tokenization function
TokenizerFunctionInterface* createTokenizer_punctuation( ErrorBufferInterface* errorhnd);

}//namespace
#endif

