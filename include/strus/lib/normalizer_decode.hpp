/*
 * Copyright (c) 2020 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Exported functions of the strus analyzer entity decoding function library
/// \file normalizer_decode.hpp
#ifndef _STRUS_ANALYZER_NORMALIZER_DECODE_LIB_HPP_INCLUDED
#define _STRUS_ANALYZER_NORMALIZER_DECODE_LIB_HPP_INCLUDED

/// \brief strus toplevel namespace
namespace strus
{

/// \brief Forward declaration
class NormalizerFunctionInterface;
/// \brief Forward declaration
class ErrorBufferInterface;

NormalizerFunctionInterface* createNormalizer_decode_xmlent( ErrorBufferInterface* errorhnd);
NormalizerFunctionInterface* createNormalizer_decode_url( ErrorBufferInterface* errorhnd);

}//namespace
#endif


