/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Exported functions of the strus analyzer vector space aggregator functions library
/// \file aggregator_vsm.hpp
#ifndef _STRUS_ANALYZER_AGGREGATOR_VSM_LIB_HPP_INCLUDED
#define _STRUS_ANALYZER_AGGREGATOR_VSM_LIB_HPP_INCLUDED

/// \brief strus toplevel namespace
namespace strus
{

/// \brief Forward declaration
class AggregatorFunctionInterface;
/// \brief Forward declaration
class ErrorBufferInterface;

/// \brief Get the aggregator function type for the cosine measure normalization factor
/// \return the aggregator function
AggregatorFunctionInterface* createAggregator_sumSquareTf( ErrorBufferInterface* errorhnd);

}//namespace
#endif

