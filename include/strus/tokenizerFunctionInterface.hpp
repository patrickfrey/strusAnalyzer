/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Interface for a tokenizer function type
/// \file tokenizerFunctionInterface.hpp
#ifndef _STRUS_ANALYZER_TOKENIZER_FUNCTION_INTERFACE_HPP_INCLUDED
#define _STRUS_ANALYZER_TOKENIZER_FUNCTION_INTERFACE_HPP_INCLUDED
#include <vector>
#include <string>

/// \brief strus toplevel namespace
namespace strus {

/// \brief Forward declaration
class TokenizerFunctionInstanceInterface;
/// \brief Forward declaration
class TextProcessorInterface;

/// \class TokenizerFunctionInterface
/// \brief Interface for a tokenizer function
class TokenizerFunctionInterface
{
public:
	/// \brief Destructor
	virtual ~TokenizerFunctionInterface(){}

	/// \brief Create a parameterizable tokenizer function instance
	/// \param[in] args arguments for the tokenizer function
	/// \param[in] tp text processor reference (for loading resources)
	/// \param[in] errorhnd analyzer error buffer interface for reporting exeptions and errors
	virtual TokenizerFunctionInstanceInterface* createInstance(
			const std::vector<std::string>& args,
			const TextProcessorInterface* tp) const=0;

	/// \brief Get a description of the function for user help
	/// \return the description
	virtual const char* getDescription() const=0;
};

}//namespace
#endif


