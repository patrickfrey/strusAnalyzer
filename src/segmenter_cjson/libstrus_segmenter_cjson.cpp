/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Exported functions of the strus JSON segmenter library
#include "strus/lib/segmenter_cjson.hpp"
#include "strus/errorBufferInterface.hpp"
#include "segmenter.hpp"
#include "jsonParser.hpp"
#include "strus/base/string_conv.hpp"
#include "strus/base/local_ptr.hpp"
#include "strus/base/dll_tags.hpp"
#include "private/internationalization.hpp"
#include "private/errorUtils.hpp"
#include "private/textEncoder.hpp"

static bool g_intl_initialized = false;

using namespace strus;

DLL_PUBLIC SegmenterInterface* strus::createSegmenter_cjson( ErrorBufferInterface* errorhnd)
{
	try
	{
		if (!g_intl_initialized)
		{
			strus::initMessageTextDomain();
			g_intl_initialized = true;
		}
		return new Segmenter( errorhnd);
	}
	CATCH_ERROR_ARG1_MAP_RETURN( _TXT("cannot create '%s' segmenter: %s"), "cjson", *errorhnd, 0);
}

static std::vector<std::string> splitJsonDocumentList_UTF8( const std::string& content)
{
	std::vector<std::string> rt;
	try
	{
		char const* curstr = content.c_str();
		char const* nextstr = strus::jsonSkipNextDocumentStart( curstr);
		while (nextstr)
		{
			rt.push_back( std::string( curstr, nextstr-curstr));
			curstr = nextstr;
			nextstr = strus::jsonSkipNextDocumentStart( curstr);
		}
		rt.push_back( curstr);
		return rt;
	}
	catch (const std::runtime_error& err)
	{
		throw strus::runtime_error( _TXT("syntax error in document number %d: %s"), (int)rt.size()+1, err.what());
	}
}

DLL_PUBLIC std::vector<std::string> strus::splitJsonDocumentList( const std::string& encoding, const std::string& content, ErrorBufferInterface* errorhnd)
{
	try
	{
		std::vector<std::string> rt;
		if (!g_intl_initialized)
		{
			strus::initMessageTextDomain();
			g_intl_initialized = true;
		}
		strus::local_ptr<strus::utils::TextEncoderBase> decoder;
		strus::local_ptr<strus::utils::TextEncoderBase> encoder;
		if (!encoding.empty() && !strus::caseInsensitiveEquals( encoding, "utf-8"))
		{
			decoder.reset( utils::createTextDecoder( encoding.c_str()));
			encoder.reset( utils::createTextEncoder( encoding.c_str()));
			std::string content_utf8 = decoder->convert( content.c_str(), content.size(), true/*eof*/);
			rt = splitJsonDocumentList_UTF8( content_utf8);
			std::vector<std::string>::iterator ri = rt.begin(), re = rt.end();
			for (; ri != re; ++ri)
			{
				std::string content_orig = encoder->convert( ri->c_str(), ri->size(), true/*eof*/);
				*ri = content_orig;
			}
		}
		else
		{
			rt = splitJsonDocumentList_UTF8( content);
		}
		return rt;
	}
	CATCH_ERROR_ARG1_MAP_RETURN( _TXT("cannot split %s document list: %s"), "JSON", *errorhnd, std::vector<std::string>());
}


