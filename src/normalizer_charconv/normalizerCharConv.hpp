/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef _STRUS_NORMALIZER_CHARACTER_CONVERSIONS_HPP_INCLUDED
#define _STRUS_NORMALIZER_CHARACTER_CONVERSIONS_HPP_INCLUDED
#include "strus/normalizerFunctionInterface.hpp"
#include "strus/normalizerFunctionInstanceInterface.hpp"
#include "strus/normalizerFunctionContextInterface.hpp"
#include "private/internationalization.hpp"
#include <string>
#include <vector>
#include <map>

namespace strus
{
/// \brief Forward declaration
class ErrorBufferInterface;

class LowercaseNormalizerFunction
	:public NormalizerFunctionInterface
{
public:
	explicit LowercaseNormalizerFunction( ErrorBufferInterface* errorhnd_)
		:m_errorhnd(errorhnd_){}

	virtual NormalizerFunctionInstanceInterface* createInstance( const std::vector<std::string>& args, const TextProcessorInterface*) const;

	virtual const char* getDescription() const
	{
		return _TXT("Normalizer mapping all characters to lowercase.");
	}

private:
	ErrorBufferInterface* m_errorhnd;
};

class UppercaseNormalizerFunction
	:public NormalizerFunctionInterface
{
public:
	explicit UppercaseNormalizerFunction( ErrorBufferInterface* errorhnd_)
		:m_errorhnd(errorhnd_){}

	virtual NormalizerFunctionInstanceInterface* createInstance( const std::vector<std::string>& args, const TextProcessorInterface*) const;

	virtual const char* getDescription() const
	{
		return _TXT("Normalizer mapping all characters to uppercase.");
	}

private:
	ErrorBufferInterface* m_errorhnd;
};

class DiacriticalNormalizerFunction
	:public NormalizerFunctionInterface
{
public:
	explicit DiacriticalNormalizerFunction( ErrorBufferInterface* errorhnd_)
		:m_errorhnd(errorhnd_){}

	virtual NormalizerFunctionInstanceInterface* createInstance( const std::vector<std::string>& args, const TextProcessorInterface*) const;

	virtual const char* getDescription() const
	{
		return _TXT("Normalizer mapping all diacritical characters to ascii. The language is passed as first argument (currently only german 'de' and english 'en' supported).");
	}

private:
	ErrorBufferInterface* m_errorhnd;
};

class CharSelectNormalizerFunction
	:public NormalizerFunctionInterface
{
public:
	explicit CharSelectNormalizerFunction( ErrorBufferInterface* errorhnd_)
		:m_errorhnd(errorhnd_){}

	virtual NormalizerFunctionInstanceInterface* createInstance( const std::vector<std::string>& args, const TextProcessorInterface*) const;

	virtual const char* getDescription() const
	{
		return _TXT("Normalizer mapping all alpha characters to identity and all other characters to nothing. The language set is passed as first argument (currently only european 'eu' and ASCII 'ascii' supported).");
	}

private:
	ErrorBufferInterface* m_errorhnd;
};

}//namespace
#endif

