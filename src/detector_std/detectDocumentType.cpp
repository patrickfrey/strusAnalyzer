/*
 * Copyright (c) 2018 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Detect encoding and type of document
/// \file detectDocumentType.cpp
#include "detectDocumentType.hpp"
#include "private/textEncoder.hpp"
#include <cstring>
using namespace strus;

static bool isDocumentJson( char const* ci, const char* ce)
{
	static const char* tokchr = "[]{}E-+0123456789.\'\"";
	if (ci != ce && *ci == '{')
	{
		for (++ci; ci != ce && (unsigned char)*ci <= 32; ++ci){}
		if (ci != ce && *ci == '"')
		{
			for (++ci; ci != ce && (unsigned char)*ci != '"'; ++ci){}
			if (ci != ce)
			{
				for (++ci; ci != ce && (unsigned char)*ci <= 32; ++ci){}
				if (ci != ce && *ci == ':')
				{
					for (++ci; ci != ce && (unsigned char)*ci <= 32; ++ci){}
					if (ci != ce && std::strchr( tokchr, *ci) != 0)
					{
						return true;
					}
				}
			}
		}
	}
	return false;
}

static int checkDocumentTSV( char const* ci, const char* ce)
{
	unsigned int seps[2];
	unsigned int nofSeps = 0;
	unsigned int nofLines = 0;
	
	for (; ci != ce && nofLines < 2; ++ci)
	{
		switch (*ci)
		{
			case '\n':
				seps[nofLines] = nofSeps;
				nofLines++;
				nofSeps = 0;
				break;
			
			case '\t':
				nofSeps++;
				break;
		}
		if (nofLines > 2)
		{
			break;
		}
	}
	if ( nofLines >= 2)
	{
		nofSeps = seps[0];
		for (unsigned int i = 1; i < 2; i++)
		{
			if (nofSeps != seps[i])
			{
				return -1;
			}
		}
		return +1;
	}
	return 0;
}

static bool isDocumentText( char const* ci, const char* ce)
{
	for (int cidx=0; ci != ce && cidx < 1024; ++ci,++cidx)
	{
		if (*ci == 0) continue;
		if ((unsigned char)*ci < 32 && *ci != '\t' && *ci != '\n' && *ci != '\r') return false;
	}
	return true;
}

struct Encoding
{
	const char* name;
	int namelen;
};

static Encoding g_encodings[] = {
	{"utf-8",5},
	{"utf-16be",8},
	{"utf-16le",8},
	{"utf-32be",8},
	{"utf-32le",8},
	{"utf-16",6},
	{"utf-32",6},
	{"isolatin-1",10},
	{0,0}};

static const char* getEncoding( const char* src, char eb)
{
	int ii=0;
	for (; g_encodings[ii].name; ++ii)
	{
		if (0==std::memcmp( g_encodings[ii].name, src, g_encodings[ii].namelen) && src[ g_encodings[ii].namelen] == eb) return g_encodings[ii].name;
	}
	return 0;
}

static bool isDocumentXML( char const* ci, const char* ce, const char*& encoding)
{
	enum State
	{
		ParseStart,
		ParseXMLHeader0,
		ParseXMLHeader,
		ParseXMLTag
	};
	char hdr[ 1024];
	std::size_t hdrsize = 0;
	State state = ParseStart;

	for (; ci != ce; ++ci)
	{
		if (*ci == 0) continue;

		switch (state)
		{
			case ParseStart:
				if (*ci == '<')
				{
					state = ParseXMLHeader0;
				}
				else
				{
					return false;
				}
				break;

			case ParseXMLHeader0:
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
					hdr[ hdrsize] = '\0';
					char const* ei = std::strstr( hdr, "encoding=");
					if (ei)
					{
						ei += 9;
						char eb = *ei++;
						if (eb == '\"' || eb == '\'')
						{
							encoding = getEncoding( ei, eb);
						}
						else
						{
							return false;
						}
					}
					return true;
				}
				else if ((unsigned char)*ci > 32)
				{
					if (hdrsize+1 >= sizeof(hdr)) return false;
					hdr[ hdrsize++] = (*ci | 32);
				}
				break;

			case ParseXMLTag:
				if (*ci == '<') return false;
				if (*ci == '>')
				{
					return true;
				}
				break;
		}
	}
	return false;
}

DocumentType strus::detectDocumentType( const char* src, std::size_t srcsize, bool eof)
{
	int chk;
	std::size_t BOMsize = 0;
	char const* encoding = utils::detectBOM( src, srcsize, BOMsize);
	char const* si = src+BOMsize;
	const char* se = src+srcsize-BOMsize;

	if (isDocumentJson( si, se))
	{
		return DocumentType( DocumentType::MimeJSON, "application/json", encoding);
	}
	else if (isDocumentXML( si, se, encoding))
	{
		return DocumentType( DocumentType::MimeXML, "application/xml", encoding);
	}
	else if (0<(chk=checkDocumentTSV( si, se)))
	{
		return DocumentType( DocumentType::MimeTSV, "text/tab-separated-values", encoding);
	}
	else if (isDocumentText( si, se))
	{
		return DocumentType( DocumentType::MimeTEXT, "text/plain", encoding);
	}
	else
	{
		return DocumentType();
	}
}



