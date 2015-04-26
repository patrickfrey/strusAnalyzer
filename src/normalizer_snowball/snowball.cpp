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
#include "strus/normalizerFunctionInterface.hpp"
#include "strus/normalizerFunctionInstanceInterface.hpp"
#include "strus/normalizerExecutionContextInterface.hpp"
#include "snowball.hpp"
#include "libstemmer.h"
#include "textwolf/charset_utf8.hpp"
#include "textwolf/cstringiterator.hpp"
#include "private/utils.hpp"
#include <stdexcept>

using namespace strus;

class StemNormalizerExecutionContext
	:public NormalizerExecutionContextInterface
{
public:
	StemNormalizerExecutionContext( const struct sb_stemmer* stemmer_)
		:m_stemmer(stemmer_)
		,m_env( sb_stemmer_create_env( stemmer_))
	{}

	virtual ~StemNormalizerExecutionContext()
	{
		sb_stemmer_delete_env( m_stemmer, m_env);
	}

	virtual std::string normalize( const char* src, std::size_t srcsize)
	{
		const sb_symbol* res
			= sb_stemmer_stem_threadsafe(
				m_stemmer, m_env, (const sb_symbol*)src, srcsize);
		if (!res) throw std::bad_alloc();
		std::size_t len = (std::size_t)sb_stemmer_length_threadsafe( m_env);

		return std::string( (const char*)res, len);
	}

private:
	const struct sb_stemmer* m_stemmer;
	struct SN_env* m_env;
};


class StemNormalizerFunctionInstance
	:public NormalizerFunctionInstanceInterface
{
public:
	StemNormalizerFunctionInstance( const std::string& language)
	{
		std::string language_lo = utils::tolower( language);
		m_stemmer = sb_stemmer_new_threadsafe( language_lo.c_str(), 0/*UTF-8 is default*/);
		if (!m_stemmer)
		{
			throw std::runtime_error( std::string( "language '") + language + "' unknown for snowball stemmer");
		}
	}

	virtual ~StemNormalizerFunctionInstance()
	{
		if (m_stemmer)
		{
			sb_stemmer_delete( m_stemmer);
		}
	}

	virtual NormalizerExecutionContextInterface* createExecutionContext() const
	{
		return new StemNormalizerExecutionContext( m_stemmer);
	}

private:
	struct sb_stemmer* m_stemmer;
};


class StemNormalizerFunction
	:public NormalizerFunctionInterface
{
public:
	StemNormalizerFunction(){}

	virtual NormalizerFunctionInstanceInterface* createInstance( const std::vector<std::string>& args, const TextProcessorInterface*) const
	{
		if (args.size() != 1)
		{
			throw std::runtime_error( "illegal number of arguments passed to snowball stemmer");
		}
		return new StemNormalizerFunctionInstance( args[0]);
	}
};


const NormalizerFunctionInterface* strus::snowball_stemmer()
{
	static const StemNormalizerFunction rt;
	return &rt;
}


