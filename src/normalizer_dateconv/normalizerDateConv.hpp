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
#include "private/internationalization.hpp"
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

	virtual const char* getDescription() const
	{
		return _TXT(
			"Normalizer mapping a date to an integer. The granularity of the result is passed as first argument and alternative date formats as following arguments."
			"Returns a date time difference of a date time value to a constant base date time value (e.g. '1970-01-01') as integer."
			"The first parameter specifies the unit of the result and the constant base date time value."
			"This unit is specified as string with the granularity (one of { 'us'=microseconds, 'ms'=milliseconds, 's'=seconds, 'm'=minutes, 'h'=hours, 'd'=days })"
			"optionally followed by the base date time value. If the base date time value is not specified, then \"1970-01-01\" is assumed.");
	}

private:
	ErrorBufferInterface* m_errorhnd;
};

}//namespace
#endif

