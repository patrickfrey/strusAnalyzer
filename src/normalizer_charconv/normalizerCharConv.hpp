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
#include <string>
#include <vector>
#include <map>

namespace strus
{
/// \brief Forward declaration
class AnalyzerErrorBufferInterface;

class LowercaseNormalizerFunction
	:public NormalizerFunctionInterface
{
public:
	explicit LowercaseNormalizerFunction( AnalyzerErrorBufferInterface* errorhnd_)
		:m_errorhnd(errorhnd_){}

	virtual NormalizerFunctionInstanceInterface* createInstance( const std::vector<std::string>& args, const TextProcessorInterface*) const;

	virtual const char* getDescription() const
	{
		return "Normalizer mapping all characters to lowercase.";
	}

private:
	AnalyzerErrorBufferInterface* m_errorhnd;
};

class UppercaseNormalizerFunction
	:public NormalizerFunctionInterface
{
public:
	explicit UppercaseNormalizerFunction( AnalyzerErrorBufferInterface* errorhnd_)
		:m_errorhnd(errorhnd_){}

	virtual NormalizerFunctionInstanceInterface* createInstance( const std::vector<std::string>& args, const TextProcessorInterface*) const;

	virtual const char* getDescription() const
	{
		return "Normalizer mapping all characters to uppercase.";
	}

private:
	AnalyzerErrorBufferInterface* m_errorhnd;
};

class DiacriticalNormalizerFunction
	:public NormalizerFunctionInterface
{
public:
	explicit DiacriticalNormalizerFunction( AnalyzerErrorBufferInterface* errorhnd_)
		:m_errorhnd(errorhnd_){}

	virtual NormalizerFunctionInstanceInterface* createInstance( const std::vector<std::string>& args, const TextProcessorInterface*) const;

	virtual const char* getDescription() const
	{
		return "Normalizer mapping all diacritical characters to ascii. The language is passed as first argument and alternative date formats as following argument (currently only german 'de' and english 'en' supported).";
	}

private:
	AnalyzerErrorBufferInterface* m_errorhnd;
};

}//namespace
#endif

