/*
 * Copyright (c) 2020 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "normalizerDecode.hpp"
#include "strus/errorBufferInterface.hpp"
#include "strus/base/string_conv.hpp"
#include "private/errorUtils.hpp"
#include "private/internationalization.hpp"
#include <cstring>

using namespace strus;

class DecodeXmlEntityNormalizerInstance
	:public NormalizerFunctionInstanceInterface
{
public:
	DecodeXmlEntityNormalizerInstance( ErrorBufferInterface* errorhnd)
		:m_errorhnd(errorhnd){}

	virtual std::string normalize(
			const char* src,
			std::size_t srcsize) const
	{
		try
		{
			return strus::string_conv::decodeXmlEntities( std::string( src, srcsize));
		}
		CATCH_ERROR_ARG1_MAP_RETURN( _TXT("error in '%s' normalizer: %s"), "decode_xmlent", *m_errorhnd, 0);
	}

	virtual const char* name() const	{return "decode_xmlent";}
	virtual StructView view() const
	{
		try
		{
			return StructView()
				( "name", name())
			;
		}
		CATCH_ERROR_MAP_RETURN( _TXT("error in introspection: %s"), *m_errorhnd, StructView());
	}

private:
	ErrorBufferInterface* m_errorhnd;
};


NormalizerFunctionInstanceInterface* DecodeXmlEntityNormalizerFunction::createInstance( const std::vector<std::string>& args, const TextProcessorInterface*) const
{
	try
	{
		if (!args.empty()) throw std::runtime_error(_TXT("no arguments expected"));
		return new DecodeXmlEntityNormalizerInstance( m_errorhnd);
	}
	CATCH_ERROR_ARG1_MAP_RETURN( _TXT("error creating '%s' normalizer instance: %s"), "decode_xmlent", *m_errorhnd, 0);
}

StructView DecodeXmlEntityNormalizerFunction::view() const
{
	try
	{
		return StructView()
			("name", name())
			("description",_TXT("Normalizer decoding all XML character entities."));
	}
	CATCH_ERROR_MAP_RETURN( _TXT("error in introspection: %s"), *m_errorhnd, StructView());
}


class DecodeUrlEntityNormalizerInstance
	:public NormalizerFunctionInstanceInterface
{
public:
	DecodeUrlEntityNormalizerInstance( ErrorBufferInterface* errorhnd)
		:m_errorhnd(errorhnd){}

	virtual std::string normalize(
			const char* src,
			std::size_t srcsize) const
	{
		try
		{
			return strus::string_conv::decodeUrlEntities( std::string( src, srcsize));
		}
		CATCH_ERROR_ARG1_MAP_RETURN( _TXT("error in '%s' normalizer: %s"), "decode_url", *m_errorhnd, 0);
	}

	virtual const char* name() const	{return "decode_url";}
	virtual StructView view() const
	{
		try
		{
			return StructView()
				( "name", name())
			;
		}
		CATCH_ERROR_MAP_RETURN( _TXT("error in introspection: %s"), *m_errorhnd, StructView());
	}

private:
	ErrorBufferInterface* m_errorhnd;
};


NormalizerFunctionInstanceInterface* DecodeUrlEntityNormalizerFunction::createInstance( const std::vector<std::string>& args, const TextProcessorInterface*) const
{
	try
	{
		if (!args.empty()) throw std::runtime_error(_TXT("no arguments expected"));
		return new DecodeUrlEntityNormalizerInstance( m_errorhnd);
	}
	CATCH_ERROR_ARG1_MAP_RETURN( _TXT("error creating '%s' normalizer instance: %s"), "decode_url", *m_errorhnd, 0);
}

StructView DecodeUrlEntityNormalizerFunction::view() const
{
	try
	{
		return StructView()
			("name", name())
			("description",_TXT("Normalizer decoding all URL character entities."));
	}
	CATCH_ERROR_MAP_RETURN( _TXT("error in introspection: %s"), *m_errorhnd, StructView());
}




