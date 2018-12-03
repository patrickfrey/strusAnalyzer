/*
 * Copyright (c) 2018 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Implementation of the data built by a POS tagger
/// \file posTaggerData.cpp
#include "posTaggerData.hpp"
#include "strus/errorBufferInterface.hpp"
#include "strus/debugTraceInterface.hpp"
#include "strus/base/utf8.hpp"
#include "strus/errorCodes.hpp"
#include "strus/tokenMarkupContextInterface.hpp"
#include "private/internationalization.hpp"
#include "private/errorUtils.hpp"
#include <algorithm>
#include <stdexcept>

#define COMPONENT_NAME "POS tagger data"
#define STRUS_DBGTRACE_COMPONENT_NAME "postag"

using namespace strus;

PosTaggerData::PosTaggerData( TokenizerFunctionInstanceInterface* tokenizer_, ErrorBufferInterface* errorhnd_)
	:m_errorhnd( errorhnd_)
	,m_debugtrace(0)
	,m_tokenizer(tokenizer_)
	,m_elementHeaderMap(errorhnd_)
	,m_elementValueMap(errorhnd_)
	,m_docs()
	,m_docnoDocMap()
{
	DebugTraceInterface* dbg = m_errorhnd->debugTrace();
	if (dbg) m_debugtrace = dbg->createTraceContext( STRUS_DBGTRACE_COMPONENT_NAME);
}

PosTaggerData::~PosTaggerData()
{
	delete m_tokenizer;
	if (m_debugtrace) delete m_debugtrace;
}

int PosTaggerData::elementValueToInt( const std::string& value)
{
	int rt = m_elementValueMap.getOrCreate( value);
	if (!rt) throw std::bad_alloc();
	return rt;
}

const char* PosTaggerData::elementValue( int valueidx) const
{
	return m_elementValueMap.key( valueidx);
}

std::string PosTaggerData::elementHeaderToString( const Element::Type& type, const std::string& value)
{
	std::string rt;
	rt.push_back( (char)type);
	rt.append( value);
	return rt;
}

int PosTaggerData::elementHeaderToInt( const Element::Type& type, const std::string& tag)
{
	std::string key( elementHeaderToString( type, tag));
	int rt = m_elementHeaderMap.getOrCreate( key);
	if (!rt) throw std::bad_alloc();
	return rt;
}

PosTaggerDataInterface::Element::Type PosTaggerData::elementType( int headeridx) const
{
	const char* rec = m_elementHeaderMap.key( headeridx);
	return rec ? (PosTaggerDataInterface::Element::Type)rec[0] : PosTaggerDataInterface::Element::Marker;
}

const char* PosTaggerData::elementTag( int headeridx) const
{
	const char* rec = m_elementHeaderMap.key( headeridx);
	return rec ? (rec+1) : 0;
}

PosTaggerData::DocAssignment PosTaggerData::createDocAssignment( const std::vector<Element>& elements)
{
	DocAssignment rt;
	std::vector<Element>::const_iterator ei = elements.begin(), ee = elements.end();
	for (; ei != ee; ++ei)
	{
		rt.ar.push_back( TagAssignment( elementHeaderToInt( ei->type(), ei->tag()), elementValueToInt(ei->value()), ei->ref().empty()?0:elementValueToInt(ei->ref())));
	}
	return rt;
}

std::vector<analyzer::Token> PosTaggerData::tokenize( const char* src, std::size_t srcsize) const
{
	std::vector<analyzer::Token> rt = m_tokenizer->tokenize( src, srcsize);
	if (rt.empty() && m_errorhnd->hasError()) throw std::runtime_error( m_errorhnd->fetchError());
	return rt;
}

std::vector<PosTaggerDataInterface::Element> PosTaggerData::tokenize( const std::vector<Element>& sequence) const
{
	std::vector<Element> rt;
	std::vector<Element>::const_iterator ei = sequence.begin(), ee = sequence.end();
	for (; ei != ee; ++ei)
	{
		std::vector<analyzer::Token> tokens = tokenize( ei->value().c_str(), ei->value().size());
		std::vector<analyzer::Token>::const_iterator ti = tokens.begin(), te = tokens.end();
		if (ti != te)
		{
			rt.push_back( Element( ei->type(), ei->tag(), std::string( ei->value().c_str() + ti->origpos().ofs(), ti->origsize()), ei->ref()));
			for (++ti; ti != te; ++ti)
			{
				rt.push_back( Element( Element::BoundToPrevious, "", std::string( ei->value().c_str() + ti->origpos().ofs(), ti->origsize()), ""));
			}
		}
		else if (ei->value().empty())
		{
			rt.push_back( Element( ei->type(), ei->tag(), std::string(), ei->ref()));
		}
	}
	return rt;
}

void PosTaggerData::declareIgnoredToken( const std::string& value)
{
	try
	{
		if (std::find( m_ignoredTokens.begin(), m_ignoredTokens.end(), value) == m_ignoredTokens.end())
		{
			m_ignoredTokens.push_back( value);
		}
	}
	CATCH_ERROR_ARG1_MAP( _TXT("error declaring token to ignore in \"%s\": %s"), COMPONENT_NAME, *m_errorhnd);
}

void PosTaggerData::insert( int docno, const std::vector<Element>& elements_)
{
	try
	{
		if (docno < 0) throw strus::runtime_error_ec( ErrorCodeInvalidArgument, _TXT("docno <= 0 not allowed"));
		m_docnoDocMap[ docno] = m_docs.size();
		m_docs.push_back( createDocAssignment( tokenize( elements_)));
	}
	CATCH_ERROR_ARG1_MAP( _TXT("error insert elements in \"%s\": %s"), COMPONENT_NAME, *m_errorhnd);
}

static bool compareTokenValue( const char* val, const char* tok, int toksize)
{
	int vi = 0;
	for (; vi < toksize && val[vi] && val[vi] == tok[vi]; ++vi){}
	return (!val[vi] && vi == toksize);
}

static void splitToken( std::vector<analyzer::Token>& tokens, std::vector<analyzer::Token>::iterator tokitr, int splitlen)
{
	analyzer::Position splitpos( tokitr->origpos().seg(), tokitr->origpos().ofs() + splitlen);
	analyzer::Token prefixtok( tokitr->ordpos(), tokitr->origpos(), splitlen);
	tokitr->setOrigPosition( splitpos);
	tokitr->setOrigSize( tokitr->origsize() - splitlen);
	tokens.insert( tokitr, prefixtok);
}

static std::string getSegmentExcerptString( const char* segmentptr, int segmentsize, int ofs)
{
	int srcpos = 0;
	int srcsize = 50;
	if (ofs > 30)
	{
		srcpos = ofs - 20;
		while (srcpos > 0 && strus::utf8midchr( segmentptr[ srcpos])) --srcpos;
	}
	if (srcpos + srcsize > (int)segmentsize)
	{
		srcsize = (int)segmentsize - srcpos;
	}
	else
	{
		while (srcpos + srcsize < (int)segmentsize && strus::utf8midchr( segmentptr[ srcpos + srcsize])) ++srcsize;
	}
	std::string srcstr;
	if (srcpos > 0)
	{
		srcstr.append( "...");
	}
	char const* si = segmentptr + srcpos;
	int sidx = 0;
	for (; sidx < srcsize; ++sidx,++si)
	{
		if ((unsigned char)*si <= 32)
		{
			srcstr.push_back( ' ');
		}
		else
		{
			srcstr.push_back( *si);
		}
	}
	if (srcpos + srcsize < (int)segmentsize)
	{
		srcstr.append( "...");
	}
	return srcstr;
}

static std::string getTokenListExcerptString( const char* segmentptr, const std::vector<analyzer::Token>& tokens, std::size_t tidx)
{
	std::string rt;
	if (tidx > 5)
	{
		tidx -= 5;
	}
	else
	{
		tidx = 0;
	}
	int ii=0;
	for (; ii<10 && tidx<tokens.size(); ++ii,++tidx)
	{
		if (ii) rt.append( ", ");
		rt.push_back( '[');
		rt.append( segmentptr + tokens[ tidx].origpos().ofs(), tokens[ tidx].origsize());
		rt.push_back( ']');
	}
	return rt;
}

static std::string getTagsExcerptString( const SymbolTable& symtab, const std::vector<PosTaggerData::TagAssignment>& tgar, std::size_t docidx)
{
	int pos = 0;
	int size = 10;
	if (docidx > 5)
	{
		pos = docidx - 5;
	}
	if (pos + size > (int)tgar.size())
	{
		size = (int)tgar.size() - pos;
	}
	std::string rt;
	int idx = 0;
	for (; idx < size; ++idx)
	{
		if (idx)
		{
			rt.append(", ");
		}
		rt.push_back( '[');
		rt.append( symtab.key( tgar[ pos+idx].valueidx));
		rt.push_back( ']');
	}
	return rt;
}

void PosTaggerData::markupSegment( TokenMarkupContextInterface* markupContext, int docno, int& docitr, const SegmenterPosition& segmentpos, const char* segmentptr, std::size_t segmentsize) const
{
	try
	{
		if (!docno) throw strus::runtime_error_ec( ErrorCodeInvalidArgument, _TXT("docno zero 0 not allowed"));

		std::string segment( segmentptr, segmentsize);
		std::vector<analyzer::Token> tokens = tokenize( segmentptr, segmentsize);
		if (tokens.empty()) return;

		std::map<int,int>::const_iterator di = m_docnoDocMap.find( docno);
		if (di == m_docnoDocMap.end())
		{
			throw strus::runtime_error_ec( ErrorCodeInvalidArgument, _TXT("document (docno %d) tagged undefined"), docno);
		}
		int docidx = di->second;
		const std::vector<TagAssignment>& tgar = m_docs[ docidx].ar;
		std::vector<TagAssignment>::const_iterator ai = tgar.begin() + docitr;
		struct
		{
			Element::Type type;
			const char* tag;
			const char* ref;
			int startofs;
			int endofs;
		} state = {Element::Marker,0,0,0,0};

		std::vector<analyzer::Token>::const_iterator ti = tokens.begin(), te = tokens.end();
		for (; ti != te && docitr < (int)tgar.size(); ++ti,++docitr,++ai)
		{
			// Initialize locals:
			const char* tv = segmentptr + ti->origpos().ofs();
			const char* ev = elementValue( ai->valueidx);
			Element::Type et = elementType( ai->headeridx);
			const char* eg = elementTag( ai->headeridx);
			const char* ref = 0;
			if (ai->refidx) ref = elementValue( ai->refidx);

			// Test if token as expected or if it can be ignored:
			if (!compareTokenValue( ev, tv, ti->origsize()))
			{
				std::string tokval( tv, ti->origsize());
				if (std::find( m_ignoredTokens.begin(), m_ignoredTokens.end(), tokval) != m_ignoredTokens.end())
				{
					//... not found, but declared as to ignore if not found
					--docitr,--ai;
					continue; //.. loop increment correction for document iterator, compare next token again
				}
				else if (std::find( m_ignoredTokens.begin(), m_ignoredTokens.end(), ev) != m_ignoredTokens.end())
				{
					//... not found, but declared as to ignore if not found
					--ti; //.. loop increment correction for token iterator, compare next document item again
					continue;
				}
				else
				{
					int evlen = std::strlen( ev);
					if (evlen < ti->origsize() && 0==std::memcmp( ev, tv, evlen))
					{
						// ... Here we handle the case the NLP tokenizer is splitting some tokens the tokenizer used here does not split.
						std::size_t tidx = ti - tokens.begin();
						splitToken( tokens, tokens.begin() + tidx, evlen);
						ti = tokens.begin() + tidx;
						te = tokens.end();
					}
					else
					{
						std::string srcstr = getSegmentExcerptString( segmentptr, segmentsize, ti->origpos().ofs());
						std::string docstr = getTagsExcerptString( m_elementValueMap, tgar, docitr);
						std::string tokstr = getTokenListExcerptString( segmentptr, tokens, ti - tokens.begin());
						throw strus::runtime_error( _TXT( "unexpected token '%s' in document, expected '%s' at token index %d (in document at '%s', POS tokens '%s', tokens '%s')"), tokval.c_str(), ev, docitr, srcstr.c_str(), docstr.c_str(), tokstr.c_str());
					}
				}
			}
			// Extend current scope and continue if bound to previous:
			if (et == Element::BoundToPrevious)
			{
				state.endofs = ti->origpos().ofs() + ti->origsize();
				continue;
			}
			// Close previously opened tag:
			if (state.tag)
			{
				analyzer::Position startpos( segmentpos, state.startofs);
				analyzer::Position endpos( segmentpos, state.endofs);
				if (state.type == Element::Marker)
				{
					endpos = startpos;
				}
				if (state.ref)
				{
					std::vector<analyzer::TokenMarkup::Attribute> attributes;
					attributes.push_back( analyzer::TokenMarkup::Attribute( "id", state.ref));
					markupContext->putMarkup( startpos, endpos, analyzer::TokenMarkup( state.tag, attributes), 0/*level*/);
				}
				else
				{
					markupContext->putMarkup( startpos, endpos, analyzer::TokenMarkup( state.tag), 0/*level*/);
				}
				state.tag = 0;
				state.startofs = 0;
				state.endofs = 0;
			}
			// Create new state:
			if (eg[0])
			{
				state.type = et;
				state.tag = eg;
				state.ref = ref;
				state.startofs = ti->origpos().ofs();
				state.endofs = state.startofs + ti->origsize();
			}
		}
		if (ti != te)
		{
			const char* tv = segmentptr + ti->origpos().ofs();
			std::string tok( tv, ti->origsize());
			throw strus::runtime_error( _TXT( "unexpected token '%s' after end of document"), tok.c_str());
		}
		// Close previously opened tag:
		if (state.tag)
		{
			// Close previously opened tag:
			analyzer::Position startpos( segmentpos, state.startofs);
			analyzer::Position endpos( segmentpos, state.endofs);
			if (state.type == Element::Marker)
			{
				endpos = startpos;
			}
			if (state.ref)
			{
				std::vector<analyzer::TokenMarkup::Attribute> attributes;
				attributes.push_back( analyzer::TokenMarkup::Attribute( "id", state.ref));
				markupContext->putMarkup( startpos, endpos, analyzer::TokenMarkup( state.tag, attributes), 0/*level*/);
			}
			else
			{
				markupContext->putMarkup( startpos, endpos, analyzer::TokenMarkup( state.tag), 0/*level*/);
			}
		}
	}
	CATCH_ERROR_ARG1_MAP( _TXT("error getting tags of segment in \"%s\": %s"), COMPONENT_NAME, *m_errorhnd);
}

