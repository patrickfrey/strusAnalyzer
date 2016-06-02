/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Interface for a normalizer function type
/// \file normalizerFunctionInterface.hpp
#ifndef _STRUS_ANALYZER_NORMALIZER_FUNCTION_INTERFACE_HPP_INCLUDED
#define _STRUS_ANALYZER_NORMALIZER_FUNCTION_INTERFACE_HPP_INCLUDED
#include <vector>
#include <string>

/// \brief strus toplevel namespace
namespace strus
{

/// \brief Forward declaration
class TextProcessorInterface;
/// \brief Forward declaration
class NormalizerFunctionInstanceInterface;

/// \class NormalizerFunctionInterface
/// \brief Interface for the normalizer constructor
class NormalizerFunctionInterface
{
public:
	/// \brief Destructor
	virtual ~NormalizerFunctionInterface(){}

	/// \brief Create a parameterizable normalizer function instance
	/// \param[in] args arguments for the normalizer function
	/// \param[in] tp text processor reference (for loading resources)
	/// \param[in] errorhnd analyzer error buffer interface for reporting exeptions and errors
	virtual NormalizerFunctionInstanceInterface* createInstance(
			const std::vector<std::string>& args,
			const TextProcessorInterface* tp) const=0;

	/// \brief Get a description of the function for user help
	/// \return the description
	virtual const char* getDescription() const=0;
};

}//namespace
#endif

