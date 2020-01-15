/*
 * Copyright (c) 2020 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Functions to read and process a document as tree structure
/// \file libstrus_doctree.cpp
#include "strus/lib/doctree.hpp"
#include "textwolf.hpp"
#include "strus/errorBufferInterface.hpp"
#include "strus/base/dll_tags.hpp"
#include "strus/base/string_conv.hpp"
#include "private/internationalization.hpp"
#include "private/errorUtils.hpp"
#include <cstdlib>
#include <memory>
#include <stdexcept>

static bool g_intl_initialized = false;

using namespace strus;

template <class CharsetEncoding>
static DocTree* createDocTreeXML( const CharsetEncoding& charset, char* src, std::size_t srcsize)
{
	std::vector<DocTreeRef> stk;
	typedef textwolf::XMLScanner<
			textwolf::CStringIterator,
			CharsetEncoding,
			textwolf::charset::UTF8,
			std::string
		> XMLScanner;

	textwolf::CStringIterator srciter( src, srcsize);
	XMLScanner xmlScanner( charset, srciter);

	typename XMLScanner::iterator itr = xmlScanner.begin(false);
	typename XMLScanner::iterator end = xmlScanner.end();
	std::string name;

	jmp_buf eom;
	{
		if (setjmp(eom) != 0)
		{
			throw std::runtime_error( _TXT( "unexpected end of input"));
		}
		while (itr != end)
		{
			++itr;
			const char* valstr = itr->content();
			int valsize = itr->size();
	
			switch (itr->type())
			{
				case XMLScanner::None:
				{
					throw strus::runtime_error( _TXT("unexpected element at position %u"), (unsigned int)xmlScanner.getTokenPosition());
				}
				case XMLScanner::ErrorOccurred:
				{
					const char* errstr = "";
					xmlScanner.getError( &errstr);
					throw strus::runtime_error( _TXT("error in document at position %u: %s"), (unsigned int)xmlScanner.getTokenPosition(), errstr);
				}
				case XMLScanner::Exit:
				{
					if (stk.empty()) return 0;
					if (stk.size() > 1) throw strus::runtime_error( _TXT("unexpected element at position %u: %s"), (unsigned int)xmlScanner.getTokenPosition(), _TXT("tags not balanced"));
					return stk.back().release();
				}
				case XMLScanner::HeaderStart:
				case XMLScanner::HeaderAttribName:
				case XMLScanner::HeaderAttribValue:
				case XMLScanner::HeaderEnd:
				case XMLScanner::DocAttribValue:
				case XMLScanner::DocAttribEnd:
				{
					break;///... ignored
				}
				case XMLScanner::TagAttribName:
				{
					name = std::string( valstr, valsize);
					break;
				}
				case XMLScanner::TagAttribValue:
				{
					if (stk.empty()) throw strus::runtime_error( _TXT("syntax error at position %u: %s"), (unsigned int)xmlScanner.getTokenPosition(), _TXT("unexpected attribute"));
					stk.back()->addAttr( name, std::string( valstr, valsize));
					break;
				}
				case XMLScanner::OpenTag:
				{
					if (!stk.empty() && !stk.back()->value().empty())
					{
						DocTreeRef valueChld( new DocTree( ""));
						valueChld->setValue( stk.back()->value());
						stk.back()->setValue("");
						stk.back()->addChld( valueChld);
					}
					stk.push_back( new DocTree( std::string( valstr, valsize)));
					break;
				}
				case XMLScanner::CloseTagIm:
				case XMLScanner::CloseTag:
				{
					if (stk.empty()) throw strus::runtime_error( _TXT("syntax error at position %u: %s"), (unsigned int)xmlScanner.getTokenPosition(), _TXT("unexpected close tag"));
					DocTreeRef node = stk.back();
					stk.pop_back();
					stk.back()->addChld( node);
					break;
				}
				case XMLScanner::Content:
				{
					
					if (stk.empty()) throw strus::runtime_error( _TXT("syntax error at position %u: %s"), (unsigned int)xmlScanner.getTokenPosition(), _TXT("unexpected value without an enclosing tag"));
					if (!stk.back()->value().empty()) throw strus::runtime_error( _TXT("logic error at position %u: %s"), (unsigned int)xmlScanner.getTokenPosition(), _TXT("unexpected content"));
					if (!stk.back()->chld().empty())
					{
						DocTreeRef valueChld( new DocTree( ""));
						valueChld->setValue( std::string( valstr, valsize));
						stk.back()->addChld( valueChld);
					}
					else
					{
						stk.back()->setValue( std::string( valstr, valsize));
					}
				}
			}
		}
	}
	if (stk.empty()) return 0;
	if (stk.size() > 1) throw strus::runtime_error( _TXT("unexpected element at position %u: %s"), (unsigned int)xmlScanner.getTokenPosition(), _TXT("tags not balanced"));
	return stk.back().release();
}

DLL_PUBLIC DocTree* strus::createDocTree_xml( const char* encoding, char* src, std::size_t srcsize, ErrorBufferInterface* errorhnd)
{
	try
	{
		if (!g_intl_initialized)
		{
			strus::initMessageTextDomain();
			g_intl_initialized = true;
		}
		typedef textwolf::charset::UTF8 UTF8;
		typedef textwolf::charset::UTF16<textwolf::charset::ByteOrder::BE> UTF16BE;
		typedef textwolf::charset::UTF16<textwolf::charset::ByteOrder::LE> UTF16LE;
		typedef textwolf::charset::UCS2<textwolf::charset::ByteOrder::BE> UCS2BE;
		typedef textwolf::charset::UCS2<textwolf::charset::ByteOrder::LE> UCS2LE;
		typedef textwolf::charset::UCS4<textwolf::charset::ByteOrder::BE> UCS4BE;
		typedef textwolf::charset::UCS4<textwolf::charset::ByteOrder::LE> UCS4LE;
		typedef textwolf::charset::IsoLatin IsoLatin;
		unsigned char codepage = 1;
	
		if (!encoding || !encoding[0])
		{
			return createDocTreeXML( UTF8(), src, srcsize);
		}
		else
		{
			if (strus::caseInsensitiveStartsWith( encoding, "IsoLatin")
			||  strus::caseInsensitiveStartsWith( encoding, "ISO-8859"))
			{
				char const* cc = encoding + 8;
				if (*cc == '-')
				{
					++cc;
					if (*cc >= '1' && *cc <= '9' && cc[1] == '\0')
					{
						codepage = *cc - '0';
					}
					else
					{
						throw strus::runtime_error( _TXT("parse error in character set encoding: '%s'"), encoding);
					}
				}
				return createDocTreeXML( IsoLatin(codepage), src, srcsize);
			}
			else if (strus::caseInsensitiveEquals( encoding, "UTF-8"))
			{
				return createDocTreeXML( UTF8(), src, srcsize);
			}
			else if (strus::caseInsensitiveEquals( encoding, "UTF-16")
			||       strus::caseInsensitiveEquals( encoding, "UTF-16BE"))
			{
				return createDocTreeXML( UTF16BE(), src, srcsize);
			}
			else if (strus::caseInsensitiveEquals( encoding, "UTF-16LE"))
			{
				return createDocTreeXML( UTF16LE(), src, srcsize);
			}
			else if (strus::caseInsensitiveEquals( encoding, "UCS-2")
			||       strus::caseInsensitiveEquals( encoding, "UCS-2BE"))
			{
				return createDocTreeXML( UCS2BE(), src, srcsize);
			}
			else if (strus::caseInsensitiveEquals( encoding, "UCS-2LE"))
			{
				return createDocTreeXML( UCS2LE(), src, srcsize);
			}
			else if (strus::caseInsensitiveEquals( encoding, "UCS-4")
			||       strus::caseInsensitiveEquals( encoding, "UCS-4BE")
			||       strus::caseInsensitiveEquals( encoding, "UTF-32")
			||       strus::caseInsensitiveEquals( encoding, "UTF-32BE"))
			{
				return createDocTreeXML( UCS4BE(), src, srcsize);
			}
			else if (strus::caseInsensitiveEquals( encoding, "UCS-4LE")
			||       strus::caseInsensitiveEquals( encoding, "UTF-32LE"))
			{
				return createDocTreeXML( UCS4LE(), src, srcsize);
			}
			else
			{
				throw strus::runtime_error( _TXT("only support for UTF-8,UTF-16BE,UTF-16LE,UTF-32BE,UCS-4BE,UTF-32LE,UCS-4LE and ISO-8859 (code pages 1 to 9) as character set encoding"));
			}
		}
	}
	CATCH_ERROR_ARG1_MAP_RETURN( _TXT("failed to create '%s' document tree: %s"), "XML", *errorhnd, 0);
}


