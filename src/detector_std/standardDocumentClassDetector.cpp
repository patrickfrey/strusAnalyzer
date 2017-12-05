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
#include "private/errorUtils.hpp"
#include "private/internationalization.hpp"
#include "private/textEncoder.hpp"
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

static void initDocumentClass( analyzer::DocumentClass& dclass, const char* mimeType, const char* encoding)
{
	dclass.setMimeType( mimeType);
	if (encoding)
	{
		dclass.setEncoding( encoding);
	}
}


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

static int checkDocumentTSV( const char* ci, const char* ce)
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

static bool isDocumentText( const char* ci, const char* ce)
{
	for (; ci != ce; ++ci)
	{
		if (*ci == 0) continue;
		if ((unsigned char)*ci < 32 && *ci != '\t' && *ci != '\n' && *ci != '\r') return false;
	}
	return true;
}

bool StandardDocumentClassDetector::detect( analyzer::DocumentClass& dclass, const char* contentBegin, std::size_t contentBeginSize, bool isComplete) const
{
	try
	{
		char const* ci = contentBegin;
		char const* ce = contentBegin + contentBeginSize;
		std::string hdr;
		State state = ParseStart;
		std::size_t BOMsize = 0;
		std::string encoding_buf;
		const char* encoding = utils::detectBOM( contentBegin, contentBeginSize, BOMsize);

		for (ci+=BOMsize; ci != ce; ++ci)
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
						if (!encoding)
						{
							encoding = strus::utils::detectCharsetEncoding( contentBegin+BOMsize, contentBeginSize-BOMsize);
						}
						// Try to find out if its JSON:
						for (; ci != ce && (unsigned char)*ci < 32; ++ci){}
						if (ci != ce && *ci == '{' && isDocumentJson(ci,ce))
						{
							initDocumentClass( dclass, "application/json", encoding);
							return true;
						}
						ci = contentBegin + BOMsize;
						int tsvcheck = checkDocumentTSV( ci, ce);
						if (tsvcheck > 0)
						{
							initDocumentClass( dclass, "text/tab-separated-values", encoding);
							return true;
						}
						else if (tsvcheck < 0 && isDocumentText( ci, ce))
						{
							initDocumentClass( dclass, "text/plain", encoding);
							return true;
						}
						// Give up:
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
						initDocumentClass( dclass, "application/xml", encoding);
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
						initDocumentClass( dclass, "application/xml", encoding);
						return true;
					}
					break;
			}
		}
		return false;
	}
	CATCH_ERROR_MAP_RETURN( _TXT("error in standard document class detector: %s"), *m_errorhnd, false);
}



