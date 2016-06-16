/*
 * Copyright (c) 2016 Andreas Baumann <mail@andreasbaumann.cc>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Exported functions of the strus analyzer tokenization per language library
/// \file tokenizer_textcat.hpp
#ifndef _STRUS_ANALYZER_TOKENIZER_TEXTCAT_LIB_HPP_INCLUDED
#define _STRUS_ANALYZER_TOKENIZER_TEXTCAT_LIB_HPP_INCLUDED

/// \brief strus toplevel namespace
namespace strus
{

/// \brief Forward declaration
class TokenizerFunctionInterface;
/// \brief Forward declaration
class ErrorBufferInterface;

/// \brief Get the tokenizer type that creates the tokenization of words in a recognized language
/// \return the tokenization function
TokenizerFunctionInterface* createTokenizer_textcat( ErrorBufferInterface* errorhnd);

}//namespace
#endif

