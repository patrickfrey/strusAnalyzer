/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef _STRUS_SEGMENTER_MARKUP_CONTEXT_TEXTWOLF_HPP_INCLUDED
#define _STRUS_SEGMENTER_MARKUP_CONTEXT_TEXTWOLF_HPP_INCLUDED
#include "strus/segmenterMarkupContextInterface.hpp"
#include "strus/errorBufferInterface.hpp"
#include "private/errorUtils.hpp"
#include "private/internationalization.hpp"
#include "segmenter.hpp"
#include "private/xpathAutomaton.hpp"
#include "textwolf/char.hpp"
#include "textwolf/charset.hpp"
#include "textwolf/sourceiterator.hpp"
#include "textwolf/xmlprinter.hpp"
#include "textwolf/xmlscanner.hpp"
#include "textwolf/textscanner.hpp"
#include <cstdlib>
#include <vector>
#include <map>
#include <string>
#include <algorithm>
#include <iostream>
#include <setjmp.h>

#undef STRUS_LOWLEVEL_DEBUG

namespace strus
{

template <class CharsetEncoding>
class SegmenterMarkupContext
	:public SegmenterMarkupContextInterface
{
public:
	explicit SegmenterMarkupContext( ErrorBufferInterface* errorhnd_, const std::string& source_, const CharsetEncoding& charset_=CharsetEncoding())
		:m_source(source_)
		,m_charset(charset_)
		,m_markups()
		,m_segmentmap()
		,m_errorhnd(errorhnd_)
	{
		load();
		m_segmentitr = m_segmentmap.end();
	}

	virtual ~SegmenterMarkupContext()
	{}

	virtual bool getNext( SegmenterPosition& segpos, const char*& segment, std::size_t& segmentsize)
	{
		try
		{
			if (m_segmentitr != m_segmentmap.end() && m_segmentitr->first == segpos)
			{
				++m_segmentitr;
			}
			else if (segpos == 0)
			{
				m_segmentitr = m_segmentmap.begin();
			}
			else
			{
				m_segmentitr = m_segmentmap.upper_bound( segpos);
			}
			if (m_segmentitr == m_segmentmap.end()) return false;
			segpos = m_segmentitr->first;
			segment = m_strings.c_str() + m_segmentitr->second.segidx;
			segmentsize = m_segmentitr->second.segsize;
			return true;
		}
		CATCH_ERROR_ARG1_MAP_RETURN( _TXT("error in get next for markup context of '%s' segmenter: %s"), "textwolf", *m_errorhnd, false);
	}
	virtual unsigned int segmentSize( const SegmenterPosition& segpos)
	{
		if (m_segmentitr == m_segmentmap.end() || m_segmentitr->first != segpos)
		{
			m_segmentitr = m_segmentmap.upper_bound( segpos);
			if (m_segmentitr == m_segmentmap.end())
			{
				return 0;
			}
		}
		return m_segmentitr->second.segsize;
	}

	struct MarkupElement
	{
		enum Type {OpenTag,AttributeName,AttributeValue,CloseTag};
		Type type;
		std::size_t origpos;	//< position of the markup in the original source
		std::size_t idx;	//< index element in unsorted list of markups to make sort stable
		std::string value;	//< value of the markup element

		static const char* typeName( Type t)
		{
			static const char* ar[] = {"OpenTag","AttributeName","AttributeValue","CloseTag"};
			return ar[t];
		}

		MarkupElement( Type type_, std::size_t origpos_, std::size_t idx_, const std::string& value_)
			:type(type_),origpos(origpos_),idx(idx_),value(value_){}
#if __cplusplus >= 201103L
		MarkupElement( MarkupElement&& ) = default;
		MarkupElement( const MarkupElement& ) = default;
		MarkupElement& operator= ( MarkupElement&& ) = default;
		MarkupElement& operator= ( const MarkupElement& ) = default;
#else
		MarkupElement( const MarkupElement& o)
			:type(o.type),origpos(o.origpos),idx(o.idx),value(o.value){}
#endif

		bool operator < (const MarkupElement& o) const
		{
			if (origpos < o.origpos) return true;
			if (origpos > o.origpos) return false;
			if (idx < o.idx) return true;
			if (idx > o.idx) return false;
			return false;
		}
	};

	virtual std::string tagName( const SegmenterPosition& segpos) const
	{
		try
		{
			typename SegmentMap::const_iterator segitr = findSegment( segpos);
			if (segitr == m_segmentmap.end())
			{
				throw std::runtime_error( _TXT("segment with this position not defined"));
			}
			else
			{
				std::size_t tagidx = segitr->second.tagidx;
				return std::string( m_strings.c_str()+tagidx);
			}
		}
		CATCH_ERROR_ARG1_MAP_RETURN( _TXT("error in get tag name of '%s' segmenter markup context: %s"), "textwolf", *m_errorhnd, std::string());
	}

	virtual int tagLevel( const SegmenterPosition& segpos) const
	{
		try
		{
			typename SegmentMap::const_iterator segitr = findSegment( segpos);
			if (segitr == m_segmentmap.end())
			{
				throw std::runtime_error( _TXT("segment with this position not defined"));
			}
			else
			{
				return segitr->second.taglevel;
			}
		}
		CATCH_ERROR_ARG1_MAP_RETURN( _TXT("error in get tag hierarchy level of '%s' segmenter markup context: %s"), "textwolf", *m_errorhnd, 0);
	}

	virtual void putOpenTag(
			const SegmenterPosition& segpos,
			std::size_t ofs,
			const std::string& name)
	{
		std::size_t origpos = getOrigPosition( segpos, ofs);
		m_markups.push_back( MarkupElement( MarkupElement::OpenTag, origpos, m_markups.size(), name));
	}

	virtual void putCloseTag(
			const SegmenterPosition& segpos,
			std::size_t ofs,
			const std::string& name)
	{
		std::size_t origpos = getOrigPosition( segpos, ofs);
		m_markups.push_back( MarkupElement( MarkupElement::CloseTag, origpos, m_markups.size(), name));
	}

	virtual void putAttribute(
			const SegmenterPosition& segpos,
			std::size_t ofs,
			const std::string& name,
			const std::string& value)
	{
		try
		{
			typename SegmentMap::const_iterator segitr = findSegment( segpos);
			if (segitr == m_segmentmap.end())
			{
				throw std::runtime_error( _TXT("segment with this position not defined or it cannot be used for markup because it is not of type content"));
			}
			std::size_t origpos = getOrigPosition( segpos, ofs);
			if (m_markups.size() && m_markups.back().origpos == origpos && (m_markups.back().type == MarkupElement::OpenTag || m_markups.back().type == MarkupElement::AttributeValue))
			{
				m_markups.push_back( MarkupElement( MarkupElement::AttributeName, origpos, m_markups.size(), name));
				m_markups.push_back( MarkupElement( MarkupElement::AttributeValue, origpos, m_markups.size(), value));
			}
			else
			{
				m_markups.push_back( MarkupElement( MarkupElement::AttributeName, segitr->second.origpos, m_markups.size(), name));
				m_markups.push_back( MarkupElement( MarkupElement::AttributeValue, segitr->second.origpos, m_markups.size(), value));
			}
		}
		CATCH_ERROR_ARG1_MAP( _TXT("error in put attribute of '%s' segmenter markup context: %s"), "textwolf", *m_errorhnd);
	}

	virtual std::string getContent() const
	{
		try
		{
			std::vector<MarkupElement> markups = m_markups;
			std::sort( markups.begin(), markups.end());
			std::string rt;

			typedef textwolf::XMLPrinter<CharsetEncoding,textwolf::charset::UTF8,std::string> MyXMLPrinter;
			MyXMLPrinter printer( true);

			typename std::vector<MarkupElement>::const_iterator mi = markups.begin(), me = markups.end();
			std::size_t prevpos = 0;
			typename MarkupElement::Type prevtype = MarkupElement::CloseTag;
			for (; mi != me; ++mi)
			{
				if (mi->origpos > prevpos)
				{
					if (prevtype != MarkupElement::CloseTag)
					{
						printer.printValue( "", 0, rt);
					}
					rt.append( m_source.c_str() + prevpos, mi->origpos - prevpos);
				}
				switch (mi->type)
				{
					case MarkupElement::OpenTag:
						printer.printOpenTag( mi->value.c_str(), mi->value.size(), rt);
						break;
					case MarkupElement::AttributeName:
						printer.printToBuffer( ' ', rt);
						printer.printToBuffer( mi->value.c_str(), mi->value.size(), rt);
						printer.printToBuffer( '=', rt);
						++mi;
						if (mi == me)
						{
							--mi;
							printer.printToBufferAttributeValue( "", 0, rt);
						}
						else
						{
							printer.printToBufferAttributeValue( mi->value.c_str(), mi->value.size(), rt);
						}
						break;
					case MarkupElement::AttributeValue:
						throw std::runtime_error( _TXT("logic error: unexpected attribute value"));
					case MarkupElement::CloseTag:
					{
						bool foundOpenTag = false;
						typename std::vector<MarkupElement>::const_iterator ma = mi;
						for (;ma != markups.begin() && (ma-1)->origpos == mi->origpos; --ma)
						{
							if ((ma-1)->type == MarkupElement::OpenTag && (ma-1)->value == mi->value)
							{
								printer.printCloseTag( rt);
								foundOpenTag = true;
								break;
							}
						}
						if (!foundOpenTag)
						{
							printer.printToBuffer( "</", 2, rt);
							printer.printToBuffer( mi->value.c_str(), mi->value.size(), rt);
							printer.printToBuffer( '>', rt);
						}
						break;
					}
				}
				prevtype = mi->type;
				prevpos = mi->origpos;
				if (printer.lasterror())
				{
					throw strus::runtime_error(_TXT("error printing XML: %s"), printer.lasterror());
				}
			}
			if (prevtype != MarkupElement::CloseTag)
			{
				printer.printValue( "", 0, rt);
			}
			rt.append( m_source.c_str() + prevpos, m_source.size() - prevpos);
			return rt;
		}
		CATCH_ERROR_ARG1_MAP_RETURN( _TXT("error in get next for markup context of '%s' segmenter: %s"), "textwolf", *m_errorhnd, std::string());
	}

private:
	struct SegmentDef
	{
		std::size_t segidx;	//< index of processed segment in m_strings
		std::size_t segsize;	//< size of processed segment in m_strings
		std::size_t tagidx;	//< index of processed tagname for error messages in m_strings
		std::size_t origpos;	//< position in original source
		int taglevel;		//< level of segment in tag hierarchy

		SegmentDef( std::size_t segidx_, std::size_t segsize_, std::size_t tagidx_, std::size_t origpos_, int taglevel_)
			:segidx(segidx_),segsize(segsize_),tagidx(tagidx_),origpos(origpos_),taglevel(taglevel_){}
#if __cplusplus >= 201103L
		SegmentDef( SegmentDef&& ) = default;
		SegmentDef( const SegmentDef& ) = default;
		SegmentDef& operator= ( SegmentDef&& ) = default;
		SegmentDef& operator= ( const SegmentDef& ) = default;
#else
		SegmentDef( const SegmentDef& o)
			:segidx(o.segidx),segsize(o.segsize),tagidx(o.tagidx),origpos(o.origpos),taglevel(o.taglevel){}
#endif
	};
	typedef std::map<SegmenterPosition,SegmentDef> SegmentMap;

private:
	typename SegmentMap::const_iterator findSegment( const SegmenterPosition& segpos) const
	{
		if (m_segmentitr != m_segmentmap.end() && m_segmentitr->first == segpos)
		{
			return m_segmentitr;
		}
		else
		{
			return m_segmentmap.find( segpos);
		}
	}

	unsigned int origCharLen( char ch) const
	{
		std::string buf;
		m_charset.print( '<', buf);
		return buf.size();
	}

	void load()
	{
		typedef textwolf::XMLScanner<char*,CharsetEncoding,textwolf::charset::UTF8,std::string> MyXMLScanner;
		char* xmlitr = const_cast<char*>( m_source.c_str());

		const unsigned int cntrlCharLen = origCharLen( '>');
		MyXMLScanner scanner( m_charset, xmlitr);
		std::vector<SegmentDef> stack;
		SegmentDef segmentDef( 0, 0, 0, 0, 0);
		for (;;)
		{
			typename MyXMLScanner::ElementType et = scanner.nextItem();
#ifdef STRUS_LOWLEVEL_DEBUG
			std::cerr << "SCAN item " << scanner.getElementTypeName( et) << " '" << std::string(scanner.getItemPtr(), scanner.getItemSize()) << "'" << std::endl;
#endif
			switch (et)
			{
				case MyXMLScanner::None: continue;
				case MyXMLScanner::HeaderStart: continue;
				case MyXMLScanner::ErrorOccurred:
				{
					if (segmentDef.tagidx)
					{
						throw strus::runtime_error( _TXT("error in tag '%s': %s"), m_strings.c_str()+segmentDef.tagidx, scanner.getItemPtr());
					}
					else
					{
						throw strus::runtime_error( _TXT("error in XML (no tag context): %s"), scanner.getItemPtr());
					}
					break;
				}
				case MyXMLScanner::HeaderAttribName: continue;
				case MyXMLScanner::HeaderAttribValue: continue;
				case MyXMLScanner::HeaderEnd: continue;
				case MyXMLScanner::DocAttribValue: continue;
				case MyXMLScanner::DocAttribEnd: continue;
				case MyXMLScanner::TagAttribName: continue;
				case MyXMLScanner::TagAttribValue:
				{
					segmentDef.origpos = scanner.getPosition();
					break;
				}
				case MyXMLScanner::OpenTag:
				{
					stack.push_back( segmentDef);
					++segmentDef.taglevel;
					segmentDef.origpos = scanner.getPosition()-cntrlCharLen;
					segmentDef.tagidx = m_strings.size()+1;
					m_strings.push_back( '\0');
					m_strings.append( scanner.getItemPtr(), scanner.getItemSize());
					break;
				}
				case MyXMLScanner::CloseTagIm:
				case MyXMLScanner::CloseTag:
				{
					if (stack.empty()) throw std::runtime_error( _TXT("tags not balanced in XML"));
					segmentDef = stack.back();
					stack.pop_back();
					break;
				}
				case MyXMLScanner::Content:
				{
					segmentDef.segidx = m_strings.size()+1;
					m_strings.push_back( '\0');
					m_strings.append( scanner.getItemPtr(), segmentDef.segsize = scanner.getItemSize());
#ifdef STRUS_LOWLEVEL_DEBUG
					std::cerr << "SCAN define segment "
						  << scanner.getTokenPosition() << ": "
						  << "segidx=" << segmentDef.segidx << ","
						  << "segsize=" << segmentDef.segsize << ","
						  << "tagidx=" << segmentDef.tagidx << ","
						  << "origpos=" << segmentDef.origpos << ","
						  << "taglevel=" << segmentDef.taglevel << std::endl;
#endif
					m_segmentmap.insert( std::pair<SegmenterPosition,SegmentDef>( scanner.getTokenPosition(), segmentDef));
					break;
				}
				case MyXMLScanner::Exit:
				{
					if (!stack.empty()) throw std::runtime_error( _TXT("tags not balanced in XML"));
					return;
				}
			}
		}
	}

	std::size_t getOrigPosition( const SegmenterPosition& segpos, std::size_t ofs) const
	{
		char const* orig_strptr;
		std::size_t orig_size;
		char const* conv_strptr;
		std::size_t conv_size;
		std::size_t conv_diff;

		if (lastPositionInfo.segpos == segpos && lastPositionInfo.ofs <= ofs)
		{
			// Seek from last position in the same segment:
			if (lastPositionInfo.ofs == ofs) return lastPositionInfo.origpos;

			orig_strptr = m_source.c_str() + lastPositionInfo.origpos;
			orig_size = m_source.size() - lastPositionInfo.origpos;
			conv_strptr = m_strings.c_str() + lastPositionInfo.convpos;
			conv_size = m_strings.size() - lastPositionInfo.convpos;
			conv_diff = ofs - lastPositionInfo.ofs;
		}
		else
		{
			// Seek from start of segment:
			typename SegmentMap::const_iterator segitr = findSegment( segpos);
			if (segitr == m_segmentmap.end()) 
			{
				throw std::runtime_error( _TXT("segment with this position not defined or it is not of type content and cannot be used for markup"));
			}
			const SegmentDef& segmentDef = segitr->second;

			orig_strptr = m_source.c_str() + segpos;
			orig_size = m_source.size() - segpos;
			conv_strptr = m_strings.c_str() + segmentDef.segidx;
			conv_size = m_strings.size() - segmentDef.segidx;
			conv_diff = ofs;

			lastPositionInfo.segpos = segpos;
			lastPositionInfo.origpos = segpos;
			lastPositionInfo.convpos = segmentDef.segidx;
			lastPositionInfo.ofs = 0;
		}
		typedef textwolf::TextScanner<textwolf::CStringIterator,CharsetEncoding> OrigTextScanner;
		typedef textwolf::TextScanner<textwolf::CStringIterator,textwolf::charset::UTF8> ConvTextScanner;

		textwolf::CStringIterator orig_itr( orig_strptr, orig_size);
		OrigTextScanner orig_scanner( m_charset, orig_itr);
		textwolf::CStringIterator conv_itr( conv_strptr, conv_size);
		ConvTextScanner conv_scanner( conv_itr);

		while (conv_scanner.getPosition() < conv_diff && conv_scanner.control() != textwolf::EndOfText && orig_scanner.control() != textwolf::EndOfText)
		{
			if (*conv_scanner != *orig_scanner)
			{
				if (orig_scanner.control() == textwolf::Amp)
				{
					// ... in case of character entity, skip it
					// PF:HACK: textwolf gives us to few information about the original position, 
					//	so we have to do here something that does not belong here at all!
					++orig_scanner;
					char ascii = orig_scanner.ascii();
					for (; ascii && ascii != ';'; ++orig_scanner,ascii=orig_scanner.ascii())
					{
						if (ascii == '#' || ((ascii|32) >= 'a' && (ascii|32) <= 'z') || (ascii >= '0' && ascii <= '9') || ascii == '_' || ascii == '-') continue;
						throw std::runtime_error( _TXT( "bad entity in XML"));
					}
				}
				else
				{
					throw std::runtime_error( _TXT( "corrupt data in segmenter markup context, converted not equal original"));
				}
				++conv_scanner;
				++orig_scanner;
			}
			else if (conv_scanner.control() == textwolf::Amp)
			{
				// ... special case character entity '&amp;'
				// PF:HACK: textwolf gives us to few information about the original position, 
				//	so we have to do here something that does not belong here at all!
				++orig_scanner;
				++conv_scanner;
				int state = 1;
				if (orig_scanner.ascii() == 'a')
				{
					++orig_scanner;
					++state;
					if (orig_scanner.ascii() == 'm')
					{
						++orig_scanner;
						++state;
						if (orig_scanner.ascii() == 'p')
						{
							++orig_scanner;
							++state;
							if (orig_scanner.ascii() == ';')
							{
								++orig_scanner;
								++state;
							}
						}
					}
				}
				if (state < 5)
				{
					static const char* entitystr = "&amp;";
					int si = 1;
					for (; si < state && conv_scanner.ascii() == entitystr[si]; ++si,++conv_scanner){}
					if (si != state)
					{
						throw std::runtime_error( _TXT( "corrupt data in segmenter markup context, converted not equal original"));
					}
				}
			}
			else
			{
				++conv_scanner;
				++orig_scanner;
			}
		}
		if (conv_scanner.getPosition() != conv_diff) 
		{
			throw std::runtime_error( _TXT( "bad offset into this segment (out of range or splitting characters)"));
		}
		lastPositionInfo.origpos += orig_scanner.getPosition();
		lastPositionInfo.convpos += conv_diff;
		lastPositionInfo.ofs = ofs;

		return lastPositionInfo.origpos;
	}

private:
	std::string m_source;
	std::string m_strings;
	CharsetEncoding m_charset;
	std::vector<MarkupElement> m_markups;
	SegmentMap m_segmentmap;
	typename SegmentMap::const_iterator m_segmentitr;

	struct PositionInfo
	{
		SegmenterPosition segpos;
		std::size_t ofs;
		std::size_t origpos;
		std::size_t convpos;

		PositionInfo()
			:segpos(0),ofs(0),origpos(0),convpos(0){}
#if __cplusplus >= 201103L
		PositionInfo( PositionInfo&& ) = default;
		PositionInfo( const PositionInfo& ) = default;
		PositionInfo& operator= ( PositionInfo&& ) = default;
		PositionInfo& operator= ( const PositionInfo& ) = default;
#else
		PositionInfo( const PositionInfo& o)
			:segpos(o.segpos),ofs(o.ofs),origpos(o.origpos),convpos(o.convpos){}
#endif
	};
	mutable PositionInfo lastPositionInfo;
	ErrorBufferInterface* m_errorhnd;
};

}//namespace
#endif


