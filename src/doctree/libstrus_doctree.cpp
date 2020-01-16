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

#undef STRUS_LOWLEVEL_DEBUG

using namespace strus;

template <class CharsetEncoding>
static DocTree* createDocTreeXML( const CharsetEncoding& charset, const char* src, std::size_t srcsize)
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

#ifdef STRUS_LOWLEVEL_DEBUG
			std::cerr << "INPUT "
				  << XMLScanner::getElementTypeName( itr->type())
				  << " [" << std::string( valstr, valsize) << "]" << std::endl;
#endif
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
					if (stk.empty())
					{
						return node.release();
					}
					else
					{
						stk.back()->addChld( node);
					}
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

typedef textwolf::charset::UTF8 UTF8;
typedef textwolf::charset::UTF16<textwolf::charset::ByteOrder::BE> UTF16BE;
typedef textwolf::charset::UTF16<textwolf::charset::ByteOrder::LE> UTF16LE;
typedef textwolf::charset::UCS2<textwolf::charset::ByteOrder::BE> UCS2BE;
typedef textwolf::charset::UCS2<textwolf::charset::ByteOrder::LE> UCS2LE;
typedef textwolf::charset::UCS4<textwolf::charset::ByteOrder::BE> UCS4BE;
typedef textwolf::charset::UCS4<textwolf::charset::ByteOrder::LE> UCS4LE;
typedef textwolf::charset::IsoLatin IsoLatin;

DLL_PUBLIC DocTree* strus::createDocTree_xml( const char* encoding, const char* src, std::size_t srcsize, ErrorBufferInterface* errorhnd)
{
	try
	{
		if (!g_intl_initialized)
		{
			strus::initMessageTextDomain();
			g_intl_initialized = true;
		}
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

template <class CharsetEncoding>
class TextwolfXmlPrinter
{
public:
	TextwolfXmlPrinter( std::ostream& out_, const CharsetEncoding& charset_, const char* encoding_)
		:m_xmlPrinter(charset_),m_out(&out_),m_encoding(encoding_){}

	void printTree( const DocTree& tree)
	{
		start();
		printNode( tree);
	}

private:
	void start()
	{
		if (!m_xmlPrinter.printHeader( m_encoding, "yes", m_out))
		{
			throw std::runtime_error(_TXT("error printing XML header"));
		}
	}

	void printNode( const DocTree& node)
	{
		if (node.name().empty())
		{
			printNodeContent( node);
		}
		else
		{
			printOpenTag( node.name(), node.attr());
			printNodeContent( node);
			printCloseTag();
		}
	}

	void printNodeContent( const DocTree& node)
	{
		if (node.value().empty())
		{
			std::list<DocTreeRef>::const_iterator
				ci = node.chld().begin(), ce = node.chld().end();
			for (; ci != ce; ++ci)
			{
				printNode( **ci);
			}
		}
		else
		{
			printContent( node.value());
			if (!node.chld().empty())
			{
				throw std::runtime_error(_TXT("invalid tree: no sub nodes expected if content value defined"));
			}
		}
	}

	void printOpenTag( const std::string& tagname, const std::list<DocTree::Attribute> attributes)
	{
		if (!m_xmlPrinter.printOpenTag( tagname.c_str(), tagname.size(), m_out))
		{
			throw std::runtime_error(_TXT("error printing open tag"));
		}
		std::list<DocTree::Attribute>::const_iterator
			ai = attributes.begin(), ae = attributes.end();
		for (; ai != ae; ++ai)
		{
			if (!m_xmlPrinter.printAttribute( ai->name().c_str(), ai->name().size(), m_out))
			{
				throw std::runtime_error(_TXT("error printing attribute name"));
			}
			if (!m_xmlPrinter.printValue( ai->value().c_str(), ai->value().size(), m_out))
			{
				throw std::runtime_error(_TXT("error printing attribute value"));
			}
		}
	}
	void printCloseTag()
	{
		if (!m_xmlPrinter.printCloseTag( m_out))
		{
			throw std::runtime_error(_TXT("error printing close tag"));
		}
	}
	void printContent( const std::string& value)
	{
		if (!m_xmlPrinter.printValue( value.c_str(), value.size(), m_out))
		{
			throw std::runtime_error(_TXT("error printing content value"));
		}
	}

private:
	typedef textwolf::XMLPrinter<CharsetEncoding, UTF8,textwolf::OstreamOutput> XMLPrinter;

private:
	XMLPrinter m_xmlPrinter;
	textwolf::OstreamOutput m_out;
	const char* m_encoding;
};


template <class CharsetEncoding>
static void printDocTreeXML( std::ostream& out, const CharsetEncoding& charset, const char* encoding, const DocTree& tree, ErrorBufferInterface* errorhnd)
{
	TextwolfXmlPrinter<CharsetEncoding> printer( out, charset, encoding);
	printer.printTree( tree);
}

DLL_PUBLIC bool strus::printDocTree_xml( std::ostream& out, const char* encoding, const DocTree& tree, ErrorBufferInterface* errorhnd)
{
	try
	{
		if (!g_intl_initialized)
		{
			strus::initMessageTextDomain();
			g_intl_initialized = true;
		}
		unsigned char codepage = 1;
	
		if (!encoding || !encoding[0])
		{
			printDocTreeXML( out, UTF8(), encoding, tree, errorhnd);
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
				printDocTreeXML( out, IsoLatin(codepage), encoding, tree, errorhnd);
			}
			else if (strus::caseInsensitiveEquals( encoding, "UTF-8"))
			{
				printDocTreeXML( out, UTF8(), encoding, tree, errorhnd);
			}
			else if (strus::caseInsensitiveEquals( encoding, "UTF-16")
			||       strus::caseInsensitiveEquals( encoding, "UTF-16BE"))
			{
				printDocTreeXML( out, UTF16BE(), encoding, tree, errorhnd);
			}
			else if (strus::caseInsensitiveEquals( encoding, "UTF-16LE"))
			{
				printDocTreeXML( out, UTF16LE(), encoding, tree, errorhnd);
			}
			else if (strus::caseInsensitiveEquals( encoding, "UCS-2")
			||       strus::caseInsensitiveEquals( encoding, "UCS-2BE"))
			{
				printDocTreeXML( out, UCS2BE(), encoding, tree, errorhnd);
			}
			else if (strus::caseInsensitiveEquals( encoding, "UCS-2LE"))
			{
				printDocTreeXML( out, UCS2LE(), encoding, tree, errorhnd);
			}
			else if (strus::caseInsensitiveEquals( encoding, "UCS-4")
			||       strus::caseInsensitiveEquals( encoding, "UCS-4BE")
			||       strus::caseInsensitiveEquals( encoding, "UTF-32")
			||       strus::caseInsensitiveEquals( encoding, "UTF-32BE"))
			{
				printDocTreeXML( out, UCS4BE(), encoding, tree, errorhnd);
			}
			else if (strus::caseInsensitiveEquals( encoding, "UCS-4LE")
			||       strus::caseInsensitiveEquals( encoding, "UTF-32LE"))
			{
				printDocTreeXML( out, UCS4LE(), encoding, tree, errorhnd);
			}
			else
			{
				throw strus::runtime_error( _TXT("only support for UTF-8,UTF-16BE,UTF-16LE,UTF-32BE,UCS-4BE,UTF-32LE,UCS-4LE and ISO-8859 (code pages 1 to 9) as character set encoding"));
			}
		}
		return true;
	}
	CATCH_ERROR_ARG1_MAP_RETURN( _TXT("failed to print '%s' document tree: %s"), "XML", *errorhnd, false);
}


