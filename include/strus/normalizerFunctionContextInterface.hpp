/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Interface for the execution context of a normalizer function
/// \file normalizerFunctionContextInterface.hpp
#ifndef _STRUS_ANALYZER_NORMALIZER_FUNCTION_CONTEXT_INTERFACE_HPP_INCLUDED
#define _STRUS_ANALYZER_NORMALIZER_FUNCTION_CONTEXT_INTERFACE_HPP_INCLUDED
#include <vector>
#include <string>

/// \brief strus toplevel namespace
namespace strus
{

/// \brief Interface to the context (state) for the execution of a normalizer for one unit (document,query)
class NormalizerFunctionContextInterface
{
public:
	/// \brief Destructor
	virtual ~NormalizerFunctionContextInterface(){}

	/// \brief Normalization of a token
	/// \param[in] src start of the token to normalize
	/// \param[in] srcsize size of the token in bytes
	/// \return list of normalized tokens
	virtual std::string normalize( const char* src, std::size_t srcsize)=0;
};

}//namespace
#endif

