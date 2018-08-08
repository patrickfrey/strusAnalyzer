/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Exported functions of the strus analyzer trim normalization function library
/// \file normalizer_trim.hpp
#ifndef _STRUS_ANALYZER_NORMALIZER_TRIM_LIB_HPP_INCLUDED
#define _STRUS_ANALYZER_NORMALIZER_TRIM_LIB_HPP_INCLUDED

/// \brief strus toplevel namespace
namespace strus
{

/// \brief Forward declaration
class NormalizerFunctionInterface;
/// \brief Forward declaration
class ErrorBufferInterface;

/// \brief Get the normalizer that returns the input trimmed as result
/// \return the normalization function
NormalizerFunctionInterface* createNormalizer_trim( ErrorBufferInterface* errorhnd);

}//namespace
#endif

