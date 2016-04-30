/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Exported functions of the strus analyzer character conversion normalization function library
/// \file normalizer_charconv.hpp
#ifndef _STRUS_ANALYZER_NORMALIZER_CHARACTER_CONVERSIONS_LIB_HPP_INCLUDED
#define _STRUS_ANALYZER_NORMALIZER_CHARACTER_CONVERSIONS_LIB_HPP_INCLUDED

/// \brief strus toplevel namespace
namespace strus
{

/// \brief Forward declaration
class NormalizerFunctionInterface;
/// \brief Forward declaration
class ErrorBufferInterface;

/// \brief Get the normalizer that returns the lower case of the input as result
/// \return the normalization function
NormalizerFunctionInterface* createNormalizer_lowercase( ErrorBufferInterface* errorhnd);
/// \brief Get the normalizer that returns the upper case of the input as result
/// \return the normalization function
NormalizerFunctionInterface* createNormalizer_uppercase( ErrorBufferInterface* errorhnd);
/// \brief Get the normalizer that returns the conversion of diacritical characters to ascii of the input as result
/// \return the normalization function
NormalizerFunctionInterface* createNormalizer_convdia( ErrorBufferInterface* errorhnd);

}//namespace
#endif

