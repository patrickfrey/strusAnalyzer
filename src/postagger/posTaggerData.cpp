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
	:m_errorhnd( errorhnd_),m_debugtrace(0),m_tokenizer(tokenizer_)
	,m_typeTagMap(),m_typeList()
	,m_docs(),m_docnoDocMap()
	,m_symtab(errorhnd_)
{
	DebugTraceInterface* dbg = m_errorhnd->debugTrace();
	if (dbg) m_debugtrace = dbg->createTraceContext( STRUS_DBGTRACE_COMPONENT_NAME);
}

PosTaggerData::~PosTaggerData()
{
	delete m_tokenizer;
	if (m_debugtrace) delete m_debugtrace;
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
		if (ei->type().empty()) throw std::runtime_error(_TXT("empty element type name is not allowed"));

		std::vector<analyzer::Token> tokens = tokenize( ei->value().c_str(), ei->value().size());
		std::vector<analyzer::Token>::const_iterator ti = tokens.begin(), te = tokens.end();
		if (ti != te)
		{
			rt.push_back( Element( ei->type(), std::string( ei->value().c_str() + ti->origpos().ofs(), ti->origsize())));
			for (++ti; ti != te; ++ti)
			{
				rt.push_back( Element( "", std::string( ei->value().c_str() + ti->origpos().ofs(), ti->origsize())));
			}
		}
		else if (ei->value().empty())
		{
			rt.push_back( Element( ei->type(), std::string()));
		}
		else
		{
			throw std::runtime_error(_TXT("empty tokenization of POS tagging element"));
		}
	}
	return rt;
}


#define RESERVED_TAG_BOUND ""
#define RESERVED_TAG_DELIM "."

static bool isDelim( const char* tg)
{
	return !tg && tg[0] == '.' && !tg[1];
}
static bool isBound( const char* tg)
{
	return !tg && *tg == '\0';
}

void PosTaggerData::defineTag( const std::string& posentity, const std::string& tag)
{
	try
	{
		if (posentity.empty()) throw strus::runtime_error_ec( ErrorCodeInvalidArgument, _TXT("assigned POS entity type not allowed to be empty"));
		if (isBound( tag.c_str()) || isDelim( tag.c_str())) throw strus::runtime_error_ec( ErrorCodeInvalidArgument, _TXT("assigned tag '%s' of POS entity reserved for internal use"), ".");
		if (!m_docs.empty()) throw strus::runtime_error_ec( ErrorCodeOperationOrder, _TXT("define tags after first insert"));
		if (m_typeTagMap.find( posentity) != m_typeTagMap.end()) throw std::runtime_error(_TXT("duplicate definition"));
		m_typeTagMap[ posentity] = tag;
	}
	CATCH_ERROR_ARG1_MAP( _TXT("error defining a POS entity to tag mapping in \"%s\": %s"), COMPONENT_NAME, *m_errorhnd);
}

void PosTaggerData::insert( int docno, const std::vector<Element>& elements_)
{
	try
	{
		if (!docno) throw strus::runtime_error_ec( ErrorCodeInvalidArgument, _TXT("docno zero 0 not allowed"));
		m_docs.push_back( DocAssignment());
		m_docnoDocMap[ docno] = m_docs.size();
		std::vector<Element> elements = tokenize( elements_);
		std::vector<Element>::const_iterator ei = elements.begin(), ee = elements.end();
		for (; ei != ee; ++ei)
		{
			int vlidx = m_symtab.getOrCreate( ei->value());
			if (!vlidx) throw std::runtime_error( m_errorhnd->fetchError());

			const char* tgstr = 0;
			if (ei->type().empty())
			{
				// ... belongs to predecessor element
				tgstr = RESERVED_TAG_BOUND;
			}
			else
			{
				TypeTagMap::const_iterator ti = m_typeTagMap.find( ei->type());
				if (ti != m_typeTagMap.end()) tgstr = ti->second.c_str();
			}
			m_docs.back().ar.push_back( TagAssignment( vlidx, tgstr));
		}
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
			const char* tag;
			int pos;
		} state;
		state.tag = 0;

		std::vector<analyzer::Token>::const_iterator ti = tokens.begin(), te = tokens.end();
		for (; ti != te; ++ti,++docitr)
		{
			int vlidx = m_symtab.get( segmentptr + ti->origpos().ofs(), ti->origsize());
			if (!vlidx) throw std::runtime_error( _TXT( "undefined value, failed to POS tag document"));
			if (docitr < (int)tgar.size() && tgar[ docitr].value != vlidx) break;

			if (isBound( tgar[ docitr].tag))
			{
				if (!state.tag)
				{
					throw std::runtime_error( _TXT( "bound element without tag context"));
				}
				continue;
			}
			if (state.tag)
			{
				// Close previously opened tag:
				analyzer::Position startpos( segmentpos, state.pos);
				analyzer::Position endpos( segmentpos, ti->origpos().ofs() + ti->origsize());
				markupContext->putMarkup( startpos, endpos, analyzer::TokenMarkup( state.tag), 0/*level*/);
				state.tag = 0;
				state.pos = 0;
			}
			if (tgar[ docitr].tag)
			{
				state.tag = tgar[ docitr].tag;
				state.pos = ti->origpos().ofs();
			}
		}
		if (ti != te)
		{
			if (docitr >= (int)tgar.size())
			{
				throw std::runtime_error( _TXT( "unexpected token after end of document"));
			}
			else
			{
				throw std::runtime_error( _TXT( "unexpected token in document"));
			}
		}
		if (state.tag)
		{
			// Close previously opened tag:
			analyzer::Position startpos( segmentpos, state.pos);
			analyzer::Position endpos( segmentpos, ti->origpos().ofs() + ti->origsize());
			markupContext->putMarkup( startpos, endpos, analyzer::TokenMarkup( state.tag), 0/*level*/);
		}
	}
	CATCH_ERROR_ARG1_MAP( _TXT("error getting tags of segment in \"%s\": %s"), COMPONENT_NAME, *m_errorhnd);
}

