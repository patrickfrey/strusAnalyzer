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
#include <iostream>
#include <setjmp.h>

#define STRUS_LOWLEVEL_DEBUG

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
		CATCH_ERROR_MAP_ARG1_RETURN( _TXT("error in get next for markup context of '%s' segmenter: %s"), "textwolf", *m_errorhnd, false);
	}

	struct MarkupElement
	{
		enum Type {OpenTag,AttributeName,AttributeValue,CloseTag};
		Type type;
		std::size_t pos;
		std::size_t idx;
		std::string value;

		static const char* typeName( Type t)
		{
			static const char* ar[] = {"OpenTag","AttributeName","AttributeValue","CloseTag"};
			return ar[t];
		}

		MarkupElement( Type type_, std::size_t pos_, std::size_t idx_, const std::string& value_)
			:type(type_),pos(pos_),idx(idx_),value(value_){}
		MarkupElement( const MarkupElement& o)
			:type(o.type),pos(o.pos),idx(o.idx),value(o.value){}

		bool operator < (const MarkupElement& o) const
		{
			if (pos < o.pos) return true;
			if (pos > o.pos) return false;
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
				throw strus::runtime_error(_TXT("segment with this position not defined"));
			}
			else
			{
				std::size_t tagidx = segitr->second.tagidx;
				return std::string( m_strings.c_str()+tagidx);
			}
		}
		CATCH_ERROR_MAP_ARG1_RETURN( _TXT("error in get tag name of '%s' segmenter markup context: %s"), "textwolf", *m_errorhnd, std::string());
	}

	virtual int tagLevel( const SegmenterPosition& segpos) const
	{
		try
		{
			typename SegmentMap::const_iterator segitr = findSegment( segpos);
			if (segitr == m_segmentmap.end())
			{
				throw strus::runtime_error(_TXT("segment with this position not defined"));
			}
			else
			{
				return segitr->second.taglevel;
			}
		}
		CATCH_ERROR_MAP_ARG1_RETURN( _TXT("error in get tag hierarchy level of '%s' segmenter markup context: %s"), "textwolf", *m_errorhnd, 0);
	}

	virtual void putOpenTag(
			const SegmenterPosition& segpos,
			std::size_t ofs,
			const std::string& name)
	{
		std::size_t pos = getOrigPosition( segpos, ofs);
		m_markups.push_back( MarkupElement( MarkupElement::OpenTag, pos, m_markups.size(), name));
	}

	virtual void putCloseTag(
			const SegmenterPosition& segpos,
			std::size_t ofs,
			const std::string& name)
	{
		std::size_t pos = getOrigPosition( segpos, ofs);
		m_markups.push_back( MarkupElement( MarkupElement::CloseTag, pos, m_markups.size(), name));
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
				throw strus::runtime_error(_TXT("segment with this position not defined"));
			}
			std::size_t pos = getOrigPosition( segpos, ofs);
			if (m_markups.size() && m_markups.back().pos == pos && (m_markups.back().type == MarkupElement::OpenTag || m_markups.back().type == MarkupElement::AttributeValue))
			{
				m_markups.push_back( MarkupElement( MarkupElement::AttributeName, pos, m_markups.size(), name));
				m_markups.push_back( MarkupElement( MarkupElement::AttributeValue, pos, m_markups.size(), value));
			}
			else
			{
				m_markups.push_back( MarkupElement( MarkupElement::AttributeName, segitr->second.attrpos, m_markups.size(), name));
				m_markups.push_back( MarkupElement( MarkupElement::AttributeValue, segitr->second.attrpos, m_markups.size(), value));
			}
		}
		CATCH_ERROR_MAP_ARG1( _TXT("error in put attribute of '%s' segmenter markup context: %s"), "textwolf", *m_errorhnd);
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
				if (mi->pos > prevpos)
				{
					if (prevtype != MarkupElement::CloseTag)
					{
						printer.printValue( "", 0, rt);
					}
					rt.append( m_source.c_str() + prevpos, mi->pos - prevpos);
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
						throw strus::runtime_error(_TXT("logic error: unexpected attribute value"));
					case MarkupElement::CloseTag:
					{
						bool foundOpenTag = false;
						typename std::vector<MarkupElement>::const_iterator ma = mi;
						for (;ma != markups.begin() && (ma-1)->pos == mi->pos; --ma)
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
				prevpos = mi->pos;
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
		CATCH_ERROR_MAP_ARG1_RETURN( _TXT("error in get next for markup context of '%s' segmenter: %s"), "textwolf", *m_errorhnd, std::string());
	}

private:
	struct SegmentDef
	{
		std::size_t segidx;
		std::size_t segsize;
		std::size_t tagidx;
		std::size_t attrpos;
		int taglevel;

		SegmentDef( std::size_t segidx_, std::size_t segsize_, std::size_t tagidx_, std::size_t attrpos_, int taglevel_)
			:segidx(segidx_),segsize(segsize_),tagidx(tagidx_),attrpos(attrpos_),taglevel(taglevel_){}
		SegmentDef( const SegmentDef& o)
			:segidx(o.segidx),segsize(o.segsize),tagidx(o.tagidx),attrpos(o.attrpos),taglevel(o.taglevel){}
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

	void load()
	{
		typedef textwolf::XMLScanner<char*,CharsetEncoding,textwolf::charset::UTF8,std::string> MyXMLScanner;
		char* xmlitr = const_cast<char*>( m_source.c_str());
	
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
					segmentDef.attrpos = scanner.getPosition();
					break;
				}
				case MyXMLScanner::OpenTag:
				{
					stack.push_back( segmentDef);
					++segmentDef.taglevel;
					segmentDef.attrpos = scanner.getPosition()-1;
					segmentDef.tagidx = m_strings.size()+1;
					m_strings.push_back( '\0');
					m_strings.append( scanner.getItemPtr(), scanner.getItemSize());
					break;
				}
				case MyXMLScanner::CloseTagIm:
				case MyXMLScanner::CloseTag:
				{
					if (stack.empty()) throw strus::runtime_error(_TXT("tags not balanced in XML"));
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
						  << "attrpos=" << segmentDef.attrpos << ","
						  << "taglevel=" << segmentDef.taglevel << std::endl;
#endif
					m_segmentmap.insert( std::pair<SegmenterPosition,SegmentDef>( scanner.getTokenPosition(), segmentDef));
					break;
				}
				case MyXMLScanner::Exit:
				{
					if (!stack.empty()) throw strus::runtime_error(_TXT("tags not balanced in XML"));
					return;
				}
			}
		}
	}

	std::size_t getOrigPosition( const SegmenterPosition& segpos, std::size_t ofs) const
	{
		typename SegmentMap::const_iterator segitr = findSegment( segpos);
		if (segitr == m_segmentmap.end()) 
		{
			throw strus::runtime_error(_TXT("segment with this position not defined"));
		}
		if (segitr->second.segsize < ofs) 
		{
			throw strus::runtime_error(_TXT("offset in this segment out of range"));
		}
		typedef textwolf::TextScanner<char*,CharsetEncoding> OrigTextScanner;
		typedef textwolf::TextScanner<textwolf::CStringIterator,textwolf::charset::UTF8> ConvTextScanner;

		char* origitr;
		std::size_t strpos;
		std::size_t strlen;
		std::size_t posincr;
		if (lastPositionInfo.segpos == segpos && lastPositionInfo.ofs <= ofs)
		{
			// Seek from last position in the same segment:
			if (lastPositionInfo.ofs == ofs) return lastPositionInfo.pos;
			origitr = const_cast<char*>( m_source.c_str() + lastPositionInfo.pos);
			strpos = segitr->second.segidx + lastPositionInfo.ofs;
			strlen = ofs - lastPositionInfo.ofs;
			posincr = lastPositionInfo.pos;
		}
		else
		{
			// Seek from start of segment:
			origitr = const_cast<char*>( m_source.c_str() + segpos);
			strpos = segitr->second.segidx;
			strlen = ofs;
			posincr = segpos;
		}
		textwolf::CStringIterator convitr( m_strings.c_str() + strpos, strlen);

		OrigTextScanner orig_scanner( m_charset, origitr);
		ConvTextScanner conv_scanner( convitr);
		while (conv_scanner.control() != textwolf::EndOfText)
		{
			++conv_scanner;
			++orig_scanner;
		}
		lastPositionInfo.pos = orig_scanner.getPosition() + posincr;
		lastPositionInfo.segpos = segpos;
		lastPositionInfo.ofs = ofs;

		return lastPositionInfo.pos;
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
		std::size_t pos;

		PositionInfo()
			:segpos(0),ofs(0),pos(0){}
	};
	mutable PositionInfo lastPositionInfo;
	ErrorBufferInterface* m_errorhnd;
};

}//namespace
#endif


