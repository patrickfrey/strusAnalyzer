/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef _STRUS_NORMALIZER_DATE_CONVERSIONS_HPP_INCLUDED
#define _STRUS_NORMALIZER_DATE_CONVERSIONS_HPP_INCLUDED
#include "strus/normalizerFunctionInterface.hpp"
#include "strus/normalizerFunctionInstanceInterface.hpp"
#include <string>
#include <vector>
#include <map>

namespace strus
{
/// \brief Forward declaration
class ErrorBufferInterface;

class Date2IntNormalizerFunction
	:public NormalizerFunctionInterface
{
public:
	explicit Date2IntNormalizerFunction( ErrorBufferInterface* errorhnd_)
		:m_errorhnd(errorhnd_){}

	virtual NormalizerFunctionInstanceInterface* createInstance( const std::vector<std::string>& args, const TextProcessorInterface*) const;

	virtual const char* name() const	{return "date2int";}
	virtual StructView view() const;

private:
	ErrorBufferInterface* m_errorhnd;
};

}//namespace
#endif

