/*
 * Copyright (c) 2019 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Exported functions of the strus analyzer entityid normalization function library
/// \file normalizer_entityid.hpp
#ifndef _STRUS_ANALYZER_NORMALIZER_ENTITYID_LIB_HPP_INCLUDED
#define _STRUS_ANALYZER_NORMALIZER_ENTITYID_LIB_HPP_INCLUDED

/// \brief strus toplevel namespace
namespace strus
{

/// \brief Forward declaration
class NormalizerFunctionInterface;
/// \brief Forward declaration
class ErrorBufferInterface;

/// \brief Get the normalizer that returns an normalizer for language entities in text
/// \return the normalization function
NormalizerFunctionInterface* createNormalizer_entityid( ErrorBufferInterface* errorhnd);

}//namespace
#endif

