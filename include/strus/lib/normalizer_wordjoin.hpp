/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Exported functions of the strus analyzer wordjoin normalization function library
/// \file normalizer_ngram.hpp
#ifndef _STRUS_ANALYZER_NORMALIZER_WORDJOIN_LIB_HPP_INCLUDED
#define _STRUS_ANALYZER_NORMALIZER_WORDJOIN_LIB_HPP_INCLUDED

/// \brief strus toplevel namespace
namespace strus
{

/// \brief Forward declaration
class NormalizerFunctionInterface;
/// \brief Forward declaration
class ErrorBufferInterface;

/// \brief Get the normalizer that returns the words tokenized from the input joined
/// \return the normalization function
NormalizerFunctionInterface* createNormalizer_wordjoin( ErrorBufferInterface* errorhnd);

}//namespace
#endif

