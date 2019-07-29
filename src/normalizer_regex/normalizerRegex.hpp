/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Implementation of the regex normalizer function
/// \file normalizerRegex.hpp
#ifndef _STRUS_ANALYZER_NORMALIZER_FUNCTION_REGEX_IMPLEMENTATION_HPP_INCLUDED
#define _STRUS_ANALYZER_NORMALIZER_FUNCTION_REGEX_IMPLEMENTATION_HPP_INCLUDED
#include "strus/normalizerFunctionInterface.hpp"

/// \brief strus toplevel namespace
namespace strus {

/// \brief Forward declaration
class ErrorBufferInterface;

/// \class RegexNormalizerFunction
/// \brief Regular expression normalizer constructor
class RegexNormalizerFunction
	:public NormalizerFunctionInterface
{
public:
	RegexNormalizerFunction( ErrorBufferInterface* errorhnd_)
		:m_errorhnd(errorhnd_){}

	virtual ~RegexNormalizerFunction(){}

	virtual NormalizerFunctionInstanceInterface* createInstance(
			const std::vector<std::string>& args,
			const TextProcessorInterface* tp) const;

	virtual const char* name() const	{return "regex";}
	virtual StructView view() const;

private:
	ErrorBufferInterface* m_errorhnd;
};

}//namespace
#endif

