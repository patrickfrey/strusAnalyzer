/*
 * Copyright (c) 2018 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Library for adding attributes to selected tags of a document (currently only implemented for XML)
/// \file libstrus_markup_document_tags.cpp
#include "strus/lib/markup_document_tags.hpp"
#include "strus/errorBufferInterface.hpp"
#include "strus/base/local_ptr.hpp"
#include "strus/base/string_conv.hpp"
#include "private/xpathAutomaton.hpp"
#include "private/textEncoder.hpp"
#include "private/contentIteratorStm.hpp"
#include "textwolf/charset.hpp"
#include "textwolf/sourceiterator.hpp"
#include "textwolf/xmlprinter.hpp"
#include "strus/base/dll_tags.hpp"
#include "private/internationalization.hpp"
#include "private/errorUtils.hpp"

#define SEGMENTER_NAME "textwolf"
#define MODULE_NAME "markup document tags"

using namespace strus;

class MarkupInputBase
{
public:
	virtual ~MarkupInputBase(){}
	virtual std::string processContent( const std::string& source, const std::vector<DocumentTagMarkupDef>& markups) const=0;
};

template <class CharsetEncoding>
class MarkupInput
	:public MarkupInputBase
{
public:
	explicit MarkupInput( const CharsetEncoding& charset_=CharsetEncoding())
		:m_charset(charset_){}
	virtual ~MarkupInput(){}

	virtual std::string processContent( const std::string& source, const std::vector<DocumentTagMarkupDef>& markups) const
	{
		typedef textwolf::XMLScanner<textwolf::SrcIterator,CharsetEncoding,textwolf::charset::UTF8,std::string> XMLScanner;

		jmp_buf eom;
		XPathAutomaton automaton;
		std::vector<DocumentTagMarkupDef>::const_iterator mi = markups.begin(), me = markups.end();
		for (int midx=1; mi != me; ++mi,++midx)
		{
			automaton.defineSelectorExpression( midx, mi->selectexpr());
		}
		XPathAutomatonContext xpathselect( automaton.createContext());
		textwolf::SrcIterator srciter( source.c_str(), source.size(), &eom);
		XMLScanner scanner( m_charset, srciter);

		typename XMLScanner::iterator itr = scanner.begin(false);

		std::string rt;
		std::vector<strus::analyzer::DocumentAttribute> attributes;
		const TagAttributeMarkupInterface* current_markup = 0;
		std::size_t lastPos = 0;
		std::string tagname;
		bool eof = false;

		try
		{
			if (setjmp(eom) != 0)
			{
				if (!eof) throw strus::runtime_error( _TXT( "unexpected end of input in '%s' %s"), SEGMENTER_NAME, MODULE_NAME);
			}
			while (!eof)
			{
				++itr;
	
				typename XMLScanner::ElementType et = itr->type();
				if (et == XMLScanner::ErrorOccurred)
				{
					const char* errstr = "";
					scanner.getError( &errstr);
					throw strus::runtime_error( _TXT("error in textwolf XML scanner: %s"), errstr);
				}
				else if (et == XMLScanner::Exit)
				{
					eof = true;
				}
				else if (et == XMLScanner::OpenTag)
				{
					std::size_t curPos = scanner.getTokenPosition();
					rt.append( source.c_str()+lastPos, curPos-lastPos);
					lastPos = curPos;

					tagname = std::string( itr->content(), itr->size());
					attributes.clear();
					current_markup = 0;
				}
				else if (et == XMLScanner::CloseTag || et == XMLScanner::CloseTagIm || et == XMLScanner::Content)
				{
					if (current_markup && !tagname.empty())
					{
						attributes.push_back( current_markup->synthesizeAttribute( attributes));
						std::string content;
						textwolf::XMLPrinter<CharsetEncoding,textwolf::charset::UTF8,std::string> printer( m_charset, true);
						printer.printOpenTag( tagname.c_str(), tagname.size(), content);
						std::vector<strus::analyzer::DocumentAttribute>::const_iterator ai = attributes.begin(), ae = attributes.end();
						for (; ai != ae; ++ai)
						{
							printer.printAttribute( ai->name().c_str(), ai->name().size(), content);
							printer.printAttribute( ai->value().c_str(), ai->value().size(), content);
						}
						printer.exitTagContext( content);
						rt.append( content);
						lastPos = scanner.getTokenPosition();
					}
					tagname.clear();
					attributes.clear();
					current_markup = 0;
				}
				else if (et == XMLScanner::TagAttribName)
				{
					std::string attribname = std::string( itr->content(), itr->size());
					++itr;
					et = itr->type();
					if (et == XMLScanner::TagAttribValue)
					{
						std::string attribvalue = std::string( itr->content(), itr->size());
						attributes.push_back( strus::analyzer::DocumentAttribute( attribname, attribvalue));
					}
					else
					{
						throw strus::runtime_error( _TXT("expected value for attribute %s"), attribname.c_str());
					}
				}
				else
				{
					xpathselect.putElement( et, itr->content(), itr->size());
					int id;
					if (xpathselect.getNext( id))
					{
						current_markup = markups[ id-1].markup();
						while (xpathselect.getNext( id))
						{
							if (current_markup != markups[ id-1].markup())
							{
								throw std::runtime_error( _TXT( "contradicting markup expressions defined"));
							}
						}
					}
				}
			}
			rt.append( source.c_str()+lastPos, source.size()-lastPos);
			return rt;
		}
		catch (const std::runtime_error& err)
		{
			throw strus::runtime_error( _TXT("error in document at position %u: %s"), (unsigned int)scanner.getTokenPosition(), err.what());
		}
	}
private:
	CharsetEncoding m_charset;
};


static MarkupInputBase* createMarkupInput( const std::string& encoding)
{
	typedef textwolf::charset::UTF8 UTF8;
	typedef textwolf::charset::UTF16<textwolf::charset::ByteOrder::BE> UTF16BE;
	typedef textwolf::charset::UTF16<textwolf::charset::ByteOrder::LE> UTF16LE;
	typedef textwolf::charset::UCS2<textwolf::charset::ByteOrder::BE> UCS2BE;
	typedef textwolf::charset::UCS2<textwolf::charset::ByteOrder::LE> UCS2LE;
	typedef textwolf::charset::UCS4<textwolf::charset::ByteOrder::BE> UCS4BE;
	typedef textwolf::charset::UCS4<textwolf::charset::ByteOrder::LE> UCS4LE;
	typedef textwolf::charset::IsoLatin IsoLatin;
	unsigned char codepage = 1;
	
	if (encoding.empty())
	{
		return new MarkupInput<UTF8>();
	}
	else
	{
		if (strus::caseInsensitiveStartsWith( encoding, "IsoLatin")
		||  strus::caseInsensitiveStartsWith( encoding, "ISO-8859"))
		{
			char const* cc = encoding.c_str() + 8;
			if (*cc == '-')
			{
				++cc;
				if (*cc >= '1' && *cc <= '9' && cc[1] == '\0')
				{
					codepage = *cc - '0';
				}
				else
				{
					throw strus::runtime_error( _TXT("parse error in character set encoding: '%s'"), encoding.c_str());
				}
			}
			return new MarkupInput<IsoLatin>( IsoLatin(codepage));
		}
		else if (strus::caseInsensitiveEquals( encoding, "UTF-8"))
		{
			return new MarkupInput<UTF8>();
		}
		else if (strus::caseInsensitiveEquals( encoding, "UTF-16")
		||       strus::caseInsensitiveEquals( encoding, "UTF-16BE"))
		{
			return new MarkupInput<UTF16BE>();
		}
		else if (strus::caseInsensitiveEquals( encoding, "UTF-16LE"))
		{
			return new MarkupInput<UTF16LE>();
		}
		else if (strus::caseInsensitiveEquals( encoding, "UCS-2")
		||       strus::caseInsensitiveEquals( encoding, "UCS-2BE"))
		{
			return new MarkupInput<UCS2BE>();
		}
		else if (strus::caseInsensitiveEquals( encoding, "UCS-2LE"))
		{
			return new MarkupInput<UCS2LE>();
		}
		else if (strus::caseInsensitiveEquals( encoding, "UCS-4")
		||       strus::caseInsensitiveEquals( encoding, "UCS-4BE")
		||       strus::caseInsensitiveEquals( encoding, "UTF-32")
		||       strus::caseInsensitiveEquals( encoding, "UTF-32BE"))
		{
			return new MarkupInput<UCS4BE>();
		}
		else if (strus::caseInsensitiveEquals( encoding, "UCS-4LE")
		||       strus::caseInsensitiveEquals( encoding, "UTF-32LE"))
		{
			return new MarkupInput<UCS4LE>();
		}
		else
		{
			throw strus::runtime_error( "%s",  _TXT("only support for UTF-8,UTF-16BE,UTF-16LE,UTF-32BE,UCS-4BE,UTF-32LE,UCS-4LE and ISO-8859 (code pages 1 to 9) as character set encoding"));
		}
	}
}

DLL_PUBLIC std::string strus::markupDocumentTags( const analyzer::DocumentClass& documentClass, const std::string& content, const std::vector<DocumentTagMarkupDef>& markups, ErrorBufferInterface* errorhnd)
{
	try
	{
		strus::local_ptr<MarkupInputBase> markup;
		if (documentClass.defined())
		{
			if (strus::caseInsensitiveEquals( documentClass.mimeType(), "application/xml") || strus::caseInsensitiveEquals( documentClass.mimeType(), "xml"))
			{
				markup.reset( createMarkupInput( documentClass.encoding()));
			}
			else
			{
				throw strus::runtime_error( _TXT("unable to process this document format, not implemented for this function"));
			}
		}
		else
		{
			markup.reset( new MarkupInput<textwolf::charset::UTF8>());
		}
		return markup->processContent( content, markups);
	}
	CATCH_ERROR_ARG1_MAP_RETURN( _TXT("error in markup document tags of '%s' segmenter: %s"), SEGMENTER_NAME, *errorhnd, std::string());
}


