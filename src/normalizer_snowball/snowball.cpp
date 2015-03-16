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
#include "snowball.hpp"
#include "libstemmer.h"
#include "textwolf/charset_utf8.hpp"
#include "textwolf/cstringiterator.hpp"
#include "private/utils.hpp"
#include <stdexcept>

using namespace strus;

class StemNormalizer
	:public NormalizerInterface
{
public:
	StemNormalizer(){}

	class ThisArgument
		:public NormalizerInterface::Argument
	{
	public:
		ThisArgument( const std::string& language)
		{
			std::string language_lo = utils::tolower( language);
			m_stemmer = sb_stemmer_new_threadsafe( language_lo.c_str(), 0/*UTF-8 is default*/);
			if (!m_stemmer)
			{
				throw std::runtime_error( std::string( "language '") + language + "' unknown for snowball stemmer");
			}
		}

		virtual ~ThisArgument()
		{
			if (m_stemmer)
			{
				sb_stemmer_delete( m_stemmer);
			}
		}

		const struct sb_stemmer* stemmer() const
		{
			return m_stemmer;
		}

	private:
		struct sb_stemmer* m_stemmer;
	};

	class ThisContext
		:public NormalizerInterface::Context
	{
	public:
		explicit ThisContext( const ThisArgument* arg_)
			:m_stemmer(arg_->stemmer())
			,m_env( sb_stemmer_create_env( arg_->stemmer()))
			{}

		virtual ~ThisContext()
		{
			sb_stemmer_delete_env( m_stemmer, m_env);
		}

		const struct sb_stemmer* stemmer() const	{return m_stemmer;}
		struct SN_env* env()				{return m_env;}

	private:
		const struct sb_stemmer* m_stemmer;
		struct SN_env* m_env;
	};

	virtual Argument* createArgument( const TextProcessorInterface*, const std::vector<std::string>& arg) const
	{
		if (arg.size() != 1)
		{
			throw std::runtime_error( "illegal number of arguments passed to snowball stemmer");
		}
		return new ThisArgument( arg[0]);
	}

	virtual Context* createContext( const Argument* arg) const
	{
		return new ThisContext( reinterpret_cast<const ThisArgument*>( arg));
	}

	virtual std::string normalize( Context* ctx_, const char* src, std::size_t srcsize) const
	{
		ThisContext* ctx = reinterpret_cast<ThisContext*>( ctx_);
		const sb_symbol* res
			= sb_stemmer_stem_threadsafe(
				ctx->stemmer(), ctx->env(), (const sb_symbol*)src, srcsize);
		if (!res) throw std::bad_alloc();
		std::size_t len = (std::size_t)sb_stemmer_length_threadsafe( ctx->env());

		return std::string( (const char*)res, len);
	}
};

const NormalizerInterface* strus::snowball_stemmer()
{
	static const StemNormalizer rt;
	return &rt;
}


