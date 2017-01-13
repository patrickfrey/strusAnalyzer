/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "normalizer_wordjoin.hpp"
#include "strus/errorBufferInterface.hpp"
#include "private/utils.hpp"
#include "private/errorUtils.hpp"
#include "private/internationalization.hpp"
#include "private/tokenizeHelpers.hpp"
#include <cstring>

using namespace strus;

#define NORMALIZER_NAME "wordjoin"

static const char* skipToToken( char const* si, const char* se)
{
	for (; si < se && wordBoundaryDelimiter( si, se); si = skipChar( si)){}
	return si;
}

class WordJoinNormalizerInstance
	:public NormalizerFunctionInstanceInterface
{
public:
	WordJoinNormalizerInstance( const std::string& jointoken_, ErrorBufferInterface* errorhnd)
		:m_jointoken(jointoken_),m_errorhnd(errorhnd){}

	virtual std::string normalize(
			const char* src,
			std::size_t srcsize) const
	{
		try
		{
			std::string rt;
			char const* si = skipToToken( src, src+srcsize);
			const char* se = src+srcsize;
		
			for (;si < se; si = skipToToken(si,se))
			{
				const char* start = si;
				while (si < se && !wordBoundaryDelimiter( si, se))
				{
					si = skipChar( si);
				}
				if (!rt.empty())
				{
					rt.append( m_jointoken);
				}
				rt.append( start, si);
			}
			return rt;
		}
		CATCH_ERROR_MAP_ARG1_RETURN( _TXT("error in %s normalize: %s"), NORMALIZER_NAME, *m_errorhnd, std::string());
	}

private:
	std::string m_jointoken;
	ErrorBufferInterface* m_errorhnd;
};

NormalizerFunctionInstanceInterface* WordJoinNormalizerFunction::createInstance( const std::vector<std::string>& args, const TextProcessorInterface*) const
{
	try
	{
		if (args.size() > 1) throw strus::runtime_error(_TXT("too many arguments"));
		if (args.size() == 1)
		{
			return new WordJoinNormalizerInstance( args[0], m_errorhnd);
		}
		else
		{
			return new WordJoinNormalizerInstance( " ", m_errorhnd);
		}
	}
	CATCH_ERROR_MAP_ARG1_RETURN( _TXT("error in create \"%s\" normalizer instance: %s"), NORMALIZER_NAME, *m_errorhnd, 0);
}

