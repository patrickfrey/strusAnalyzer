/*
 * Copyright (c) 2018 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef _STRUS_NORMALIZER_SUBSTRING_INDEX_HPP_INCLUDED
#define _STRUS_NORMALIZER_SUBSTRING_INDEX_HPP_INCLUDED
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

class SubStringIndexNormalizerFunction
	:public NormalizerFunctionInterface
{
public:
	explicit SubStringIndexNormalizerFunction( ErrorBufferInterface* errorhnd_)
		:m_errorhnd(errorhnd_){}

	virtual NormalizerFunctionInstanceInterface* createInstance( const std::vector<std::string>& args, const TextProcessorInterface*) const;

	virtual const char* getDescription() const
	{
		return _TXT("Normalizer mapping the sub strings defined by the arguments to their indices starting from 0.");
	}

public:
	enum {MaxSubStringLength=256};

private:
	ErrorBufferInterface* m_errorhnd;
};

class SubStringMapNormalizerFunction
	:public NormalizerFunctionInterface
{
public:
	explicit SubStringMapNormalizerFunction( ErrorBufferInterface* errorhnd_)
		:m_errorhnd(errorhnd_){}

	virtual NormalizerFunctionInstanceInterface* createInstance( const std::vector<std::string>& args, const TextProcessorInterface*) const;

	virtual const char* getDescription() const
	{
		return _TXT("Normalizer mapping the sub strings defined by the arguments to their indices starting from 0.");
	}

public:
	enum {MaxSubStringLength=256};

private:
	ErrorBufferInterface* m_errorhnd;
};

}//namespace
#endif

