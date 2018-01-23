/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "strus/normalizerFunctionInterface.hpp"
#include "strus/normalizerFunctionInstanceInterface.hpp"
#include "strus/errorBufferInterface.hpp"
#include "snowball.hpp"
#include "libstemmer.h"
#include "strus/base/string_conv.hpp"
#include "textwolf/charset_utf8.hpp"
#include "textwolf/cstringiterator.hpp"
#include "private/errorUtils.hpp"
#include "private/internationalization.hpp"
#include <stdexcept>

using namespace strus;

class StemNormalizerFunctionInstance
	:public NormalizerFunctionInstanceInterface
{
public:
	StemNormalizerFunctionInstance( const std::string& language, ErrorBufferInterface* errorhnd)
	{
		m_errorhnd = errorhnd;
		std::string language_lo = string_conv::tolower( language);
		m_stemmer = sb_stemmer_new_threadsafe( language_lo.c_str(), 0/*UTF-8 is default*/);
		if (!m_stemmer)
		{
			errorhnd->report( *ErrorCode(StrusComponentAnalyzer,ErrorOperationBuildData,ErrorCauseNotImplemented), "language '%s' unknown for snowball stemmer", language.c_str());
		}
	}

	virtual ~StemNormalizerFunctionInstance()
	{
		if (m_stemmer) sb_stemmer_delete( m_stemmer);
	}

	virtual std::string normalize( const char* src, std::size_t srcsize) const
	{
		try
		{
			sb_stemmer_env env;
			sb_stemmer_UTF_8_init_env( m_stemmer, &env);
			const sb_symbol* res = sb_stemmer_stem_threadsafe( m_stemmer, &env, (const sb_symbol*)src, srcsize);
			if (!res)
			{
				return std::string( src, srcsize);
			}
			std::size_t len = (std::size_t)sb_stemmer_length_threadsafe( &env);
			return std::string( (const char*)res, len);
		}
		catch (const std::bad_alloc& )
		{
			m_errorhnd->report( *ErrorCode(StrusComponentAnalyzer,ErrorOperationBuildData,ErrorCauseOutOfMem), "memory allocation error in stemmer");
			return std::string();
		}
	}

private:
	struct sb_stemmer* m_stemmer;
	ErrorBufferInterface* m_errorhnd;
};


NormalizerFunctionInstanceInterface* StemNormalizerFunction::createInstance( const std::vector<std::string>& args, const TextProcessorInterface*) const
{
	try
	{
		if (args.size() != 1)
		{
			m_errorhnd->report( *ErrorCode(StrusComponentAnalyzer,ErrorOperationBuildData,ErrorCauseInvalidArgument), "illegal number of arguments passed to snowball stemmer");
			return 0;
		}
		return new StemNormalizerFunctionInstance( args[0], m_errorhnd);
	}
	CATCH_ERROR_MAP_RETURN( _TXT("error in stem normalizer: %s"), *m_errorhnd, 0);
}


