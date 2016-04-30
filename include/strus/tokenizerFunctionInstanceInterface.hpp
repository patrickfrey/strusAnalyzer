/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Interface for a parameterized tokenizer function
/// \file tokenizerFunctionInstanceInterface.hpp
#ifndef _STRUS_ANALYZER_TOKENIZER_FUNCTION_INSTANCE_INTERFACE_HPP_INCLUDED
#define _STRUS_ANALYZER_TOKENIZER_FUNCTION_INSTANCE_INTERFACE_HPP_INCLUDED

/// \brief strus toplevel namespace
namespace strus {

/// \brief Forward declaration
class TokenizerFunctionContextInterface;

/// \brief Interface for tokenization
class TokenizerFunctionInstanceInterface
{
public:
	/// \brief Destructor
	virtual ~TokenizerFunctionInstanceInterface(){}

	/// \brief Flag defined by tokenizer indicating that different segments defined by the tag hierarchy should be concatenated before tokenization
	/// \return true, if the argument chunks should be passed as one concatenated string, else if no
	/// \remark This flag is needed for context sensitive tokenization like for example for recognizing punctuation.
	virtual bool concatBeforeTokenize() const=0;

	/// \brief Create an instance (context for one document) for tokenization
	/// \return the created tokenizer instance (with ownership)
	virtual TokenizerFunctionContextInterface* createFunctionContext() const=0;
};

}//namespace
#endif


