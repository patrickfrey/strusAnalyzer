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

	virtual const char* name() const	{return "lc";}
	virtual StructView view() const;

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

	virtual const char* name() const	{return "uc";}
	virtual StructView view() const;

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

	virtual const char* name() const	{return "convdia";}
	virtual StructView view() const;

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

	virtual const char* name() const	{return "charselect";}
	virtual StructView view() const;

private:
	ErrorBufferInterface* m_errorhnd;
};

}//namespace
#endif

