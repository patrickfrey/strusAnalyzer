/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Standard document class detector implementation
/// \file standardDocumentClassDetector.hpp
#include "standardDocumentClassDetector.hpp"
#include "strus/analyzerErrorBufferInterface.hpp"
#include "private/errorUtils.hpp"
#include "private/internationalization.hpp"
#include <cstring>
#include <stdexcept>

using namespace strus;

enum State
{
	ParseStart,
	ParseXMLHeader0,
	ParseXMLHeader,
	ParseXMLTag
};

static const unsigned char BOM_UTF8[] = {3,0xEF,0xBB,0xBF};
static const unsigned char BOM_UTF16BE[] = {2,0xFE,0xFF};
static const unsigned char BOM_UTF16LE[] = {2,0xFF,0xFE};
static const unsigned char BOM_UTF32BE[] = {4,0,0,0xFE,0xFF};
static const unsigned char BOM_UTF32LE[] = {4,0xFF,0xFE,0,0};

const char* detectBOM( const char* str, std::size_t strsize)
{
	if (strsize < 4) return 0;
	if (std::memcmp( BOM_UTF8+1, str, BOM_UTF8[0]) == 0) return "UTF-8";
	if (std::memcmp( BOM_UTF16BE+1, str, BOM_UTF16BE[0]) == 0) return "UTF-16BE";
	if (std::memcmp( BOM_UTF16LE+1, str, BOM_UTF16LE[0]) == 0) return "UTF-16LE";
	if (std::memcmp( BOM_UTF32BE+1, str, BOM_UTF32BE[0]) == 0) return "UTF-32BE";
	if (std::memcmp( BOM_UTF32LE+1, str, BOM_UTF32LE[0]) == 0) return "UTF-32LE";
	return 0;
}

static void initDocumentClass( DocumentClass& dclass, const char* mimeType, const char* encoding, const char* BOM)
{
	dclass.setMimeType( mimeType);
	if (encoding)
	{
		dclass.setEncoding( encoding);
	}
	else if (BOM)
	{
		dclass.setEncoding( BOM);
	}
	else
	{
		dclass.setEncoding( "UTF-8"); //default
	}
}


bool StandardDocumentClassDetector::detect( DocumentClass& dclass, const char* contentBegin, std::size_t contentBeginSize) const
{
	try
	{
		char const* ci = contentBegin;
		char const* ce = contentBegin + contentBeginSize;
		std::string hdr;
		unsigned int nullCnt = 0;
		unsigned int maxNullCnt = 0;
		State state = ParseStart;
		const char* BOM = detectBOM( contentBegin, contentBeginSize);
		std::string encoding_buf;
		const char* encoding = 0;
	
		for (;ci != ce; ++ci)
		{
			if (*ci == 0)
			{
				++nullCnt;
				if (nullCnt > 3) return false;
				continue;
			}
			if (nullCnt > maxNullCnt)
			{
				maxNullCnt = nullCnt;
			}
			nullCnt = 0;
			switch (state)
			{
				case ParseStart:
					if (*ci == '<')
					{
						if (!BOM)
						{
							if (maxNullCnt == 3) BOM = "UTF-32BE";
							if (maxNullCnt == 1) BOM = "UTF-16BE";
						}
						state = ParseXMLHeader0;
					}
					else
					{
						return false;
					}
					break;
	
				case ParseXMLHeader0:
					if (!BOM)
					{
						if (maxNullCnt == 3) BOM = "UTF-32LE";
						if (maxNullCnt == 1) BOM = "UTF-16LE";
					}
					if (*ci == '?')
					{
						state = ParseXMLHeader;
					}
					else
					{
						state = ParseXMLTag;
					}
					break;
	
				case ParseXMLHeader:
					if (*ci == '<') return false;
					if (*ci == '>')
					{
						char const* ei = std::strstr( hdr.c_str(), "encoding=");
						if (ei)
						{
							ei += 9;
							if (*ei == '\"' || *ei == '\'')
							{
								char eb = *ei++;
								char const* ee = std::strchr( ei, eb);
								if (ee)
								{
									encoding_buf.append( ei, ee-ei);
									encoding = encoding_buf.c_str();
								}
							}
						}
						initDocumentClass( dclass, "text/xml", encoding, BOM);
						return true;
					}
					else if ((unsigned char)*ci > 32)
					{
						hdr.push_back(*ci);
					}
					break;
	
				case ParseXMLTag:
					if (*ci == '<') return false;
					if (*ci == '>')
					{
						initDocumentClass( dclass, "text/xml", 0, BOM);
						return true;
					}
					break;
			}
		}
		return false;
	}
	CATCH_ERROR_MAP_RETURN( _TXT("error in standard document class detector: %s"), *m_errorhnd, false);
}



