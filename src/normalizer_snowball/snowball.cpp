/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "strus/normalizerFunctionInterface.hpp"
#include "strus/normalizerFunctionInstanceInterface.hpp"
#include "strus/normalizerFunctionContextInterface.hpp"
#include "strus/analyzerErrorBufferInterface.hpp"
#include "snowball.hpp"
#include "libstemmer.h"
#include "textwolf/charset_utf8.hpp"
#include "textwolf/cstringiterator.hpp"
#include "private/utils.hpp"
#include "private/errorUtils.hpp"
#include "private/internationalization.hpp"
#include <stdexcept>

using namespace strus;

class StemNormalizerFunctionContext
	:public NormalizerFunctionContextInterface
{
public:
	StemNormalizerFunctionContext( const struct sb_stemmer* stemmer_, AnalyzerErrorBufferInterface* errorhnd)
		:m_stemmer(stemmer_)
		,m_env( sb_stemmer_create_env( stemmer_))
		,m_errorhnd(errorhnd)
	{}

	virtual ~StemNormalizerFunctionContext()
	{
		sb_stemmer_delete_env( m_stemmer, m_env);
	}

	virtual std::string normalize( const char* src, std::size_t srcsize)
	{
		const sb_symbol* res
			= sb_stemmer_stem_threadsafe(
				m_stemmer, m_env, (const sb_symbol*)src, srcsize);
		if (!res)
		{
			m_errorhnd->report( "memory allocation error in stemmer");
			return std::string();
		}
		std::size_t len = (std::size_t)sb_stemmer_length_threadsafe( m_env);
		try
		{
			return std::string( (const char*)res, len);
		}
		catch (const std::bad_alloc& )
		{
			m_errorhnd->report( "memory allocation error in stemmer");
			return std::string();
		}
	}

private:
	const struct sb_stemmer* m_stemmer;
	struct SN_env* m_env;
	AnalyzerErrorBufferInterface* m_errorhnd;
};


class StemNormalizerFunctionInstance
	:public NormalizerFunctionInstanceInterface
{
public:
	StemNormalizerFunctionInstance( const std::string& language, AnalyzerErrorBufferInterface* errorhnd)
	{
		m_errorhnd = errorhnd;
		std::string language_lo = utils::tolower( language);
		m_stemmer = sb_stemmer_new_threadsafe( language_lo.c_str(), 0/*UTF-8 is default*/);
		if (!m_stemmer)
		{
			errorhnd->report( "language '%s' unknown for snowball stemmer", language.c_str());
		}
	}

	virtual ~StemNormalizerFunctionInstance()
	{
		if (m_stemmer) sb_stemmer_delete( m_stemmer);
	}

	virtual NormalizerFunctionContextInterface* createFunctionContext() const
	{
		try
		{
			if (!m_stemmer) return 0;
			return new StemNormalizerFunctionContext( m_stemmer, m_errorhnd);
		}
		CATCH_ERROR_MAP_RETURN( _TXT("error in stem normalizer: %s"), *m_errorhnd, 0);
	}

private:
	struct sb_stemmer* m_stemmer;
	AnalyzerErrorBufferInterface* m_errorhnd;
};


NormalizerFunctionInstanceInterface* StemNormalizerFunction::createInstance( const std::vector<std::string>& args, const TextProcessorInterface*) const
{
	try
	{
		if (args.size() != 1)
		{
			m_errorhnd->report( "illegal number of arguments passed to snowball stemmer");
			return 0;
		}
		return new StemNormalizerFunctionInstance( args[0], m_errorhnd);
	}
	CATCH_ERROR_MAP_RETURN( _TXT("error in stem normalizer: %s"), *m_errorhnd, 0);
}


