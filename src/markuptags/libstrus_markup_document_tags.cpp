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
#include "strus/textProcessorInterface.hpp"
#include "strus/documentClassDetectorInterface.hpp"
#include "strus/lib/detector_std.hpp"
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

#undef STRUS_LOWLEVEL_DEBUG

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
class MarkupInputXml
	:public MarkupInputBase
{
public:
	explicit MarkupInputXml( const CharsetEncoding& charset_=CharsetEncoding())
		:m_charset(charset_){}
	virtual ~MarkupInputXml(){}

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
		std::vector<const TagAttributeMarkupInterface*> current_markups;
		std::size_t lastPos = 0;
		std::string tagname;
		std::string attribname;
		std::string attribvalue;
		int taglevel = 0;
		bool eof = false;

		try
		{
			if (setjmp(eom) != 0)
			{
				throw strus::runtime_error( _TXT( "unexpected end of input in '%s' %s"), SEGMENTER_NAME, MODULE_NAME);
			}
			while (!eof)
			{
				++itr;

				typename XMLScanner::ElementType et = itr->type();
#ifdef STRUS_LOWLEVEL_DEBUG
				std::cerr << "TOKEN " << XMLScanner::getElementTypeName( et) << ": '" << std::string( itr->content(), itr->size()) << "'" << std::endl;
#endif
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
				else if (et == XMLScanner::CloseTag || et == XMLScanner::CloseTagIm || et == XMLScanner::Content || et == XMLScanner::OpenTag)
				{
					if (et == XMLScanner::CloseTag || et == XMLScanner::CloseTagIm)
					{
						--taglevel;
					}
					if (!current_markups.empty() && !tagname.empty())
					{
						std::vector<strus::analyzer::DocumentAttribute> syn;
						std::vector<const TagAttributeMarkupInterface*>::const_iterator ci = current_markups.begin(), ce = current_markups.end();
						for (; ci != ce; ++ci)
						{
							syn.push_back( (*ci)->synthesizeAttribute( tagname, attributes));
						}
						std::vector<strus::analyzer::DocumentAttribute>::const_iterator si = syn.begin(), se = syn.end();
						for (; si != se; ++si)
						{
							std::size_t aidx = 0;
							std::vector<strus::analyzer::DocumentAttribute>::iterator ai = attributes.begin();
							while (aidx < attributes.size())
							{
								if (ai->name() == si->name())
								{
									if (si->value().empty())
									{
										ai = attributes.erase( ai);
										continue;
									}
									else
									{
										*ai = *si;
										break;
									}
								}
								++aidx;
								++ai;
							}
							if (aidx == attributes.size() && !si->value().empty())
							{
								attributes.push_back( *si);
							}
						}
						std::string content;
						textwolf::XMLPrinter<CharsetEncoding,textwolf::charset::UTF8,std::string> printer( m_charset, true);
						printer.printOpenTag( tagname.c_str(), tagname.size(), content);
						std::vector<strus::analyzer::DocumentAttribute>::const_iterator ai = attributes.begin(), ae = attributes.end();
						for (; ai != ae; ++ai)
						{
							printer.printAttribute( ai->name().c_str(), ai->name().size(), content);
							printer.printValue( ai->value().c_str(), ai->value().size(), content);
						}
						if (et == XMLScanner::CloseTagIm)
						{
							m_charset.print( '/', content);
						}
						else if (et == XMLScanner::Content)
						{
							m_charset.print( '>', content);
						}
						else if (et == XMLScanner::OpenTag)
						{
							m_charset.print( '>', content);
							m_charset.print( '<', content);
						}
						if (content.size() < CharsetEncoding::UnitSize)
						{
							throw strus::runtime_error( _TXT("data corruption in XML processing (textwolf)"));
						}
						rt.append( content.c_str()+CharsetEncoding::UnitSize, content.size()-CharsetEncoding::UnitSize);
						//... The staring '<' of the open tag has already been scanned by the textwolf STM. The token position points to
						//	the name of the tag. Therefore we append the tag and its attributes without the opening '<'.
						lastPos = scanner.getTokenPosition();
					}
					if (et == XMLScanner::OpenTag)
					{
						std::size_t curPos = scanner.getTokenPosition();
						rt.append( source.c_str()+lastPos, curPos-lastPos);
						lastPos = curPos;

						tagname = std::string( itr->content(), itr->size());
						++taglevel;
					}
					else
					{
						tagname.clear();
						if (taglevel == 0) eof = true;
					}
					attributes.clear();
					current_markups.clear();
				}
				else if (et == XMLScanner::TagAttribName)
				{
					attribname = std::string( itr->content(), itr->size());
				}
				else if (et == XMLScanner::TagAttribValue)
				{
					attribvalue = std::string( itr->content(), itr->size());
					attributes.push_back( strus::analyzer::DocumentAttribute( attribname, attribvalue));
				}
				xpathselect.putElement( et, itr->content(), itr->size());
				int elementid;
				while (xpathselect.getNext( elementid))
				{
					current_markups.push_back( markups[ elementid-1].markup());
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


static MarkupInputBase* createMarkupInputXml( const std::string& encoding)
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
		return new MarkupInputXml<UTF8>();
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
			return new MarkupInputXml<IsoLatin>( IsoLatin(codepage));
		}
		else if (strus::caseInsensitiveEquals( encoding, "UTF-8"))
		{
			return new MarkupInputXml<UTF8>();
		}
		else if (strus::caseInsensitiveEquals( encoding, "UTF-16")
		||       strus::caseInsensitiveEquals( encoding, "UTF-16BE"))
		{
			return new MarkupInputXml<UTF16BE>();
		}
		else if (strus::caseInsensitiveEquals( encoding, "UTF-16LE"))
		{
			return new MarkupInputXml<UTF16LE>();
		}
		else if (strus::caseInsensitiveEquals( encoding, "UCS-2")
		||       strus::caseInsensitiveEquals( encoding, "UCS-2BE"))
		{
			return new MarkupInputXml<UCS2BE>();
		}
		else if (strus::caseInsensitiveEquals( encoding, "UCS-2LE"))
		{
			return new MarkupInputXml<UCS2LE>();
		}
		else if (strus::caseInsensitiveEquals( encoding, "UCS-4")
		||       strus::caseInsensitiveEquals( encoding, "UCS-4BE")
		||       strus::caseInsensitiveEquals( encoding, "UTF-32")
		||       strus::caseInsensitiveEquals( encoding, "UTF-32BE"))
		{
			return new MarkupInputXml<UCS4BE>();
		}
		else if (strus::caseInsensitiveEquals( encoding, "UCS-4LE")
		||       strus::caseInsensitiveEquals( encoding, "UTF-32LE"))
		{
			return new MarkupInputXml<UCS4LE>();
		}
		else
		{
			throw strus::runtime_error( "%s",  _TXT("only support for UTF-8,UTF-16BE,UTF-16LE,UTF-32BE,UCS-4BE,UTF-32LE,UCS-4LE and ISO-8859 (code pages 1 to 9) as character set encoding"));
		}
	}
}

DLL_PUBLIC std::string strus::markupDocumentTags( const analyzer::DocumentClass& documentClass, const std::string& content, const std::vector<DocumentTagMarkupDef>& markups, const TextProcessorInterface* textproc, ErrorBufferInterface* errorhnd)
{
	try
	{
		strus::local_ptr<MarkupInputBase> impl;
		analyzer::DocumentClass dclass;
		if (documentClass.defined())
		{
			dclass = documentClass;
		}
		else
		{
			strus::local_ptr<DocumentClassDetectorInterface> detector( strus::createDetector_std( textproc, errorhnd));
			if (!detector.get())
			{
				throw strus::runtime_error( _TXT("failed to get document class detector: %s"), errorhnd->fetchError());
			}
			if (!detector->detect( dclass, content.c_str(), content.size(), true/*isComplete*/))
			{
				if (errorhnd->hasError())
				{
					throw strus::runtime_error( _TXT("failed to detect document class: %s"), errorhnd->fetchError());
				}
				else
				{
					throw strus::runtime_error( _TXT("unknown document class"));
				}
			}
		}
		if (strus::caseInsensitiveEquals( dclass.mimeType(), "application/xml") || strus::caseInsensitiveEquals( dclass.mimeType(), "xml"))
		{
			impl.reset( createMarkupInputXml( dclass.encoding()));
		}
		else
		{
			throw strus::runtime_error( _TXT("unable to process this document format, not implemented for this function"));
		}
		return impl->processContent( content, markups);
	}
	CATCH_ERROR_ARG1_MAP_RETURN( _TXT("error in markup document tags of '%s' segmenter: %s"), SEGMENTER_NAME, *errorhnd, std::string());
}


