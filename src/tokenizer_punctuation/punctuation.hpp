/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef _STRUS_TOKENIZER_PUNCTUATION_HPP_INCLUDED
#define _STRUS_TOKENIZER_PUNCTUATION_HPP_INCLUDED
#include "strus/tokenizerFunctionInterface.hpp"

namespace strus
{
/// \brief Forward declaration
class ErrorBufferInterface;

TokenizerFunctionInterface* punctuationTokenizer( ErrorBufferInterface* errorhnd);

}//namespace
#endif

