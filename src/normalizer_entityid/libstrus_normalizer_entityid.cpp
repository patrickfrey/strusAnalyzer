/*
 * Copyright (c) 2018 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "strus/lib/normalizer_entityid.hpp"
#include "strus/base/dll_tags.hpp"
#include "strus/normalizerFunctionInterface.hpp"
#include "strus/normalizerFunctionInstanceInterface.hpp"
#include "strus/errorBufferInterface.hpp"
#include "strus/base/utf8.hpp"
#include "private/internationalization.hpp"
#include "private/errorUtils.hpp"
#include "private/tokenizeHelpers.hpp"
#include <string>
#include <cstring>

using namespace strus;

#define NORMALIZER_NAME "entityid"

static const char g_quotes[] = "\"'’`;.:";

static bool isQuote( char const* ci)
{
	char const* ni = std::strchr( g_quotes, *ci);
	unsigned char cl = strus::utf8charlen(*ci);
	if (cl == 1) return ni != 0;
	for (; ni; ni+=cl,ni = std::strchr( g_quotes, *ci))
	{
		if (0==std::memcmp( ci,ni,cl)) return true;
	}
	return false;
}

class EntityIdNormalizerInstance
	:public NormalizerFunctionInstanceInterface
{
public:
	explicit EntityIdNormalizerInstance( ErrorBufferInterface* errorhnd_)
		:m_errorhnd(errorhnd_){}

	virtual std::string normalize(
			const char* src,
			std::size_t srcsize) const
	{
		try
		{
			std::string rt;
			char const* si = src;
			char const* se = src + srcsize;
			for (;si < se && (isQuote( si) || strus::whiteSpaceDelimiter(si,se)); si+=strus::utf8charlen(*si)){}
			char const* pi = strus::utf8prev( se);
			for (;si < se && (isQuote( pi) || strus::whiteSpaceDelimiter(pi,se)); se=pi,pi=strus::utf8prev( se)){}
			const char* start = si;

			int chrlen;
			while (si < se)
			{
				chrlen = strus::utf8charlen(*si);
				if (strus::wordBoundaryDelimiter( si, se))
				{
					rt.append( start, si-start);
					bool hasOnlySpace = strus::whiteSpaceDelimiter(si,se);
					for (si+=chrlen; si < se; si += chrlen)
					{
						chrlen = strus::utf8charlen(*si);
						if (strus::whiteSpaceDelimiter(si,se)) continue;
						if (strus::wordBoundaryDelimiter( si, se))
						{
							hasOnlySpace = false;
							continue;
						}
						break;
					}
					if (hasOnlySpace)
					{
						if (si < se)
						{
							rt.push_back( '_');
						}
					}
					else
					{
						rt.push_back( '-');
					}
					start = si;
				}
				else
				{
					si += chrlen;
				}
			}
			rt.append( start, se-start);
			return rt;
		}
		CATCH_ERROR_ARG1_MAP_RETURN( _TXT("error in %s normalize: %s"), NORMALIZER_NAME, *m_errorhnd, std::string());
	}

	virtual const char* name() const	{return "entityid";}
	virtual StructView view() const
	{
		try
		{
			return StructView()
				("name", name());
		}
		CATCH_ERROR_MAP_RETURN( _TXT("error in introspection: %s"), *m_errorhnd, StructView());
	}

private:
	ErrorBufferInterface* m_errorhnd;
};

class EntityIdNormalizerFunction
	:public NormalizerFunctionInterface
{
public:
	explicit EntityIdNormalizerFunction( ErrorBufferInterface* errorhnd_)
		:m_errorhnd(errorhnd_){}

	virtual NormalizerFunctionInstanceInterface* createInstance( const std::vector<std::string>& args, const TextProcessorInterface*) const
	{
		try
		{
			if (!args.empty())
			{
				m_errorhnd->report( ErrorCodeInvalidArgument, _TXT("no arguments expected for normalizer '%s'"), NORMALIZER_NAME);
				return 0;
			}
			return new EntityIdNormalizerInstance( m_errorhnd);
		}
		CATCH_ERROR_MAP_RETURN( _TXT("error creating instance of normalizer: %s"), *m_errorhnd, 0);
	}

	virtual const char* name() const	{return "entityid";}
	virtual StructView view() const
	{
		try
		{
			return StructView()
				("name", name())
				("description",_TXT("Normalizer mapping multi part strings to single entities usable as identifiers. Soes also some normalization of the way the entity parts are joined."));
		}
		CATCH_ERROR_MAP_RETURN( _TXT("error in introspection: %s"), *m_errorhnd, StructView());
	}

private:
	ErrorBufferInterface* m_errorhnd;
};


static bool g_intl_initialized = false;

DLL_PUBLIC NormalizerFunctionInterface* strus::createNormalizer_entityid( ErrorBufferInterface* errorhnd)
{
	try
	{
		if (!g_intl_initialized)
		{
			strus::initMessageTextDomain();
			g_intl_initialized = true;
		}
		return new EntityIdNormalizerFunction( errorhnd);
	}
	CATCH_ERROR_ARG1_MAP_RETURN( _TXT("cannot create \"%s\" normalizer: %s"), "entityid", *errorhnd, 0);
}

