/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Interface for a parameterized normalizer function
/// \file normalizerFunctionInstanceInterface.hpp
#ifndef _STRUS_ANALYZER_NORMALIZER_FUNCTION_INSTANCE_INTERFACE_HPP_INCLUDED
#define _STRUS_ANALYZER_NORMALIZER_FUNCTION_INSTANCE_INTERFACE_HPP_INCLUDED

/// \brief strus toplevel namespace
namespace strus
{
/// \brief Forward declaration
class NormalizerFunctionContextInterface;

/// \class NormalizerFunctionInstanceInterface
/// \brief Interface for a parameterized normalization function
class NormalizerFunctionInstanceInterface
{
public:
	/// \brief Destructor
	virtual ~NormalizerFunctionInstanceInterface(){}

	/// \brief Create an instance (context for one document) for normalization
	/// \return the created normalizer instance (with ownership)
	virtual NormalizerFunctionContextInterface* createFunctionContext() const=0;
};

}//namespace
#endif

