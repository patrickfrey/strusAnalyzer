/*
---------------------------------------------------------------------
    The C++ library strus implements basic operations to build
    a search engine for structured search on unstructured data.

    Copyright (C) 2013,2014 Patrick Frey

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

--------------------------------------------------------------------

	The latest version of strus can be found at 'http://github.com/patrickfrey/strus'
	For documentation see 'http://patrickfrey.github.com/strus'

--------------------------------------------------------------------
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

