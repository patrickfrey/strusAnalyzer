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
		rt.ar.push_back( TagAssignment( elementHeaderToInt( ei->type(), ei->tag()), elementValueToInt(ei->value())));
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
			rt.push_back( Element( ei->type(), ei->tag(), std::string( ei->value().c_str() + ti->origpos().ofs(), ti->origsize())));
			for (++ti; ti != te; ++ti)
			{
				rt.push_back( Element( Element::BoundToPrevious, "", std::string( ei->value().c_str() + ti->origpos().ofs(), ti->origsize())));
			}
		}
		else if (ei->value().empty())
		{
			rt.push_back( Element( ei->type(), ei->tag(), std::string()));
		}
		else
		{
			throw std::runtime_error(_TXT("empty tokenization of POS tagging element"));
		}
	}
	return rt;
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
			int startofs;
			int endofs;
		} state;
		state.tag = 0;
		state.startofs = 0;
		state.endofs = 0;

		std::vector<analyzer::Token>::const_iterator ti = tokens.begin(), te = tokens.end();
		for (; ti != te && docitr < (int)tgar.size(); ++ti,++docitr,++ai)
		{
			// Initialize locals:
			const char* tv = segmentptr + ti->origpos().ofs();
			const char* ev = elementValue( ai->valueidx);
			Element::Type et = elementType( ai->headeridx);
			const char* eg = elementTag( ai->headeridx);

			// Test if token as expected:
			int ei = 0;
			for (; ev[ ei]; ++ei)
			{
				if (ev[ei] != tv[ei])
				{
					std::string tok( tv, ti->origsize());
					throw strus::runtime_error( _TXT( "unexpected token '%s' in document"), tok.c_str());
				}
			}
			if (ei != ti->origsize())
			{
				std::string tok( tv, ti->origsize());
				throw strus::runtime_error( _TXT( "unexpected token '%s' in document"), tok.c_str());
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
				markupContext->putMarkup( startpos, endpos, analyzer::TokenMarkup( state.tag), 0/*level*/);
				state.tag = 0;
				state.startofs = 0;
				state.endofs = 0;
			}
			// Create new state:
			if (eg[0])
			{
				state.type = et;
				state.tag = eg;
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
			markupContext->putMarkup( startpos, endpos, analyzer::TokenMarkup( state.tag), 0/*level*/);
		}
	}
	CATCH_ERROR_ARG1_MAP( _TXT("error getting tags of segment in \"%s\": %s"), COMPONENT_NAME, *m_errorhnd);
}

