/*
 * Copyright (c) 2016 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "patternMatchContextMap.hpp"
#include "private/internationalization.hpp"
#include "strus/analyzer/patternLexem.hpp"
#include "strus/base/string_format.hpp"
#include "strus/errorBufferInterface.hpp"
#include "strus/debugTraceInterface.hpp"
#include "private/debugTraceHelpers.hpp"
#include <cstring>
#include <algorithm>
#include <limits>

using namespace strus;

#define STRUS_DBGTRACE_COMPONENT_NAME "analyzer"
#define DEBUG_OPEN( NAME) if (m_debugtrace) m_debugtrace->open( NAME);
#define DEBUG_CLOSE() if (m_debugtrace) m_debugtrace->close();
#define DEBUG_EVENT4( NAME, FMT, X1, X2, X3, X4)		if (m_debugtrace) m_debugtrace->event( NAME, FMT, X1, X2, X3, X4);
#define DEBUG_EVENT5( NAME, FMT, X1, X2, X3, X4, X5)		if (m_debugtrace) m_debugtrace->event( NAME, FMT, X1, X2, X3, X4, X5);
#define DEBUG_EVENT_STR( NAME, FMT, VAL)			if (m_debugtrace) {std::string valstr(VAL); m_debugtrace->event( NAME, FMT, valstr.c_str());}

PreProcPatternMatchContext::PreProcPatternMatchContext( const PreProcPatternMatchConfig& config, ErrorBufferInterface* errorhnd_)
	:m_config(&config)
	,m_matcher(config.matcher()->createContext())
	,m_lexer(config.lexer()->createContext())
	,m_errorhnd(errorhnd_)
	,m_debugtrace(0)
{
	if (!m_matcher.get())
	{
		throw std::runtime_error( _TXT("failed to create pattern matcher context"));
	}
	if (!m_lexer.get())
	{
		throw std::runtime_error( _TXT("failed to create pattern lexer context"));
	}
	DebugTraceInterface* dbgi = m_errorhnd->debugTrace();
	if (dbgi) m_debugtrace = dbgi->createTraceContext( STRUS_DBGTRACE_COMPONENT_NAME);
}

void PreProcPatternMatchContext::process( std::size_t segpos, const char* seg, std::size_t segsize)
{
	SegPosContentPosMap::const_iterator segi = m_segPosContentPosMap.find( segpos);
	if (segi == m_segPosContentPosMap.end())
	{
		m_segPosContentPosMap[ segpos] = m_content.size() + 1;
		m_content.push_back( '\0');
		m_content.append( seg, segsize);
	}
	DEBUG_EVENT_STR( "segment", "%s", strus::getStringContentStart( std::string( seg, segsize), 200));
	std::vector<analyzer::PatternLexem> lexems = m_lexer->match( seg, segsize);
	std::vector<analyzer::PatternLexem>::iterator li = lexems.begin(), le = lexems.end();
	DEBUG_OPEN( "input")
	for (; li != le; ++li)
	{
		li->setOrigseg( segpos);
		m_matcher->putInput( *li);
		DEBUG_EVENT5( "lexem", "%d pos %d [%d %d %d]", li->id(), li->ordpos(), li->origseg(), li->origpos(), li->origsize());
	}
	DEBUG_CLOSE()
}

std::vector<BindTerm> PreProcPatternMatchContext::fetchResults()
{
	std::vector<BindTerm> rt;
	std::vector<analyzer::PatternMatcherResult> results = m_matcher->fetchResults();
	std::vector<analyzer::PatternMatcherResult>::const_iterator ri = results.begin(), re = results.end();
	DEBUG_OPEN( "result")
	for (; ri != re; ++ri)
	{
		if (!m_config->allowCrossSegmentMatches() && ri->start_origseg() != ri->end_origseg()) continue;
		if (!m_config->patternTypeName().empty())
		{
			rt.push_back( BindTerm( ri->start_origseg(), ri->start_origpos(), ri->end_ordpos() - ri->start_ordpos(), analyzer::BindContent, m_config->patternTypeName(), ri->name()));
			DEBUG_EVENT5( "result", "[%d %d %d] %s '%s'", rt.back().seg(), rt.back().ofs(), rt.back().len(), rt.back().type().c_str(), rt.back().value().c_str());
		}
		std::vector<analyzer::PatternMatcherResultItem>::const_iterator ti = ri->items().begin(), te = ri->items().end();
		for (; ti != te; ++ti)
		{
			SegPosContentPosMap::const_iterator segi;
			segi = m_segPosContentPosMap.find( ti->start_origseg());
			if (segi == m_segPosContentPosMap.end()) throw std::runtime_error( _TXT("internal: inconsistency in data, segment position unknown"));
			std::size_t segstritr = segi->second;
			segi = m_segPosContentPosMap.find( ti->end_origseg());
			if (segi == m_segPosContentPosMap.end()) throw std::runtime_error( _TXT("internal: inconsistency in data, segment position unknown"));
			std::size_t segstrend = segi->second;

			if (segstritr == segstrend)
			{
				const char* chunk = m_content.c_str() + segstritr + ti->start_origpos();
				std::size_t chunksize = ti->end_origpos() - ti->start_origpos();
				rt.push_back( BindTerm( ti->start_origseg(), ti->start_origpos(), ti->end_ordpos() - ti->start_ordpos(), analyzer::BindContent, ti->name(), std::string( chunk, chunksize)));
				DEBUG_EVENT5( "result", "[%d %d %d] %s '%s'", rt.back().seg(), rt.back().ofs(), rt.back().len(), rt.back().type().c_str(), rt.back().value().c_str());
			}
			else
			{
				std::string value;
				segstritr += ti->start_origpos();
				while (segstritr < segstrend)
				{
					const char* chunk = m_content.c_str() + segstritr;
					value.append( chunk);
					segstritr += std::strlen( chunk)+1;
				}
				if (segstritr != segstrend)
				{
					throw std::runtime_error( _TXT("internal: inconsistency in data, segments overlapping or not in ascending order"));
				}
				value.append( m_content.c_str() + segstritr, ti->end_origpos());
				rt.push_back( BindTerm( ti->start_origseg(), ti->start_origpos(), ti->end_ordpos() - ti->start_ordpos(), analyzer::BindContent, ti->name(), value));
				DEBUG_EVENT5( "result", "[%d %d %d] %s '%s'", rt.back().seg(), rt.back().ofs(), rt.back().len(), rt.back().type().c_str(), rt.back().value().c_str());
			}
		}
	}
	DEBUG_CLOSE()
	return rt;
}

void PreProcPatternMatchContext::clear()
{
	m_matcher->reset();
	m_lexer->reset();
	m_content.clear();
	m_segPosContentPosMap.clear();
}

PostProcPatternMatchContext::PostProcPatternMatchContext( const PostProcPatternMatchConfig& config, ErrorBufferInterface* errorhnd_)
	:m_config(&config)
	,m_matcher(config.matcher()->createContext())
	,m_feeder(config.feeder())
	,m_input()
	,m_fetchResults_called(false)
	,m_errorhnd(errorhnd_)
	,m_debugtrace(0)
{
	if (!m_matcher.get())
	{
		throw std::runtime_error( _TXT("failed to create pattern matcher context"));
	}
	DebugTraceInterface* dbgi = m_errorhnd->debugTrace();
	if (dbgi) m_debugtrace = dbgi->createTraceContext( STRUS_DBGTRACE_COMPONENT_NAME);
}

void PostProcPatternMatchContext::process( const std::vector<BindTerm>& input)
{
	std::vector<BindTerm>::const_iterator bi = input.begin(), be = input.end();
	for (; bi != be; ++bi)
	{
		unsigned int lexemid = m_feeder->getLexem( bi->type());
		if (lexemid)
		{
			if (!bi->value().empty())
			{
				unsigned int symid = m_feeder->getSymbol( lexemid, bi->value());
				if (symid) m_input.push_back( BindLexem( symid, *bi));
			}
			m_input.push_back( BindLexem( lexemid, *bi));
		}
	}
}

std::vector<BindTerm> PostProcPatternMatchContext::fetchResults()
{
	if (m_fetchResults_called)
	{
		throw strus::runtime_error(_TXT("internal: PostProcPatternMatchContext::fetchResults called twice"));
	}
	m_fetchResults_called = true;

	DEBUG_OPEN( "result")
	std::sort( m_input.begin(), m_input.end());
	unsigned int prev_seg = std::numeric_limits<unsigned int>::max();
	unsigned int prev_ofs = std::numeric_limits<unsigned int>::max();
	unsigned int ordpos = 0;

	// Convert input elements:
	std::vector<analyzer::PatternLexem> pinput;
	std::vector<BindLexem>::const_iterator bi = m_input.begin(), be = m_input.end();
	for (std::size_t bidx=0; bi != be; ++bi,++bidx)
	{
		if (bi->ofs() != prev_ofs || bi->seg() != prev_seg)
		{
			// Increment ordinal position:
			++ordpos;
			prev_seg = bi->seg();
			prev_ofs = bi->ofs();
		}
		int virtpos = bidx;
		pinput.push_back( analyzer::PatternLexem( bi->id(), ordpos, bi->seg(), virtpos, 1));
	}

	// Feed input to matcher:
	DEBUG_OPEN( "input")
	std::vector<analyzer::PatternLexem>::const_iterator pi = pinput.begin(), pe = pinput.end();
	for (; pi != pe; ++pi)
	{
		if (pi->id())
		{
			int virtpos = bi->ofs();
			int origpos = m_input[ virtpos].ofs();
			DEBUG_EVENT5( "lexem", "%d pos %d [%d %d %d]", pi->id(), pi->ordpos(), pi->origseg(), origpos, pi->origsize());
			m_matcher->putInput( *pi);
		}
	}
	DEBUG_CLOSE()

	// Build result:
	std::vector<BindTerm> rt;
	std::vector<analyzer::PatternMatcherResult> results = m_matcher->fetchResults();
	std::vector<analyzer::PatternMatcherResult>::const_iterator ri = results.begin(), re = results.end();
	for (; ri != re; ++ri)
	{
		if (!m_config->allowCrossSegmentMatches() && ri->start_origseg() != ri->end_origseg()) continue;

		if (!m_config->patternTypeName().empty())
		{
			int start_virtpos = ri->start_origpos();
			int start_origpos = m_input[ start_virtpos].ofs();
			rt.push_back( BindTerm( ri->start_origseg(), start_origpos, ri->end_ordpos() - ri->start_ordpos(), analyzer::BindContent, m_config->patternTypeName(), ri->name()));
			DEBUG_EVENT5( "result", "[%d %d %d] %s '%s'", rt.back().seg(), rt.back().ofs(), rt.back().len(), rt.back().type().c_str(), rt.back().value().c_str());
		}
		std::vector<analyzer::PatternMatcherResultItem>::const_iterator ti = ri->items().begin(), te = ri->items().end();
		for (; ti != te; ++ti)
		{
			int start_virtpos = ti->start_origpos();
			int start_origpos = m_input[ start_virtpos].ofs();
			const std::string& value = m_input[ start_virtpos].value();
			rt.push_back( BindTerm( ti->start_origseg(), start_origpos, ti->end_ordpos() - ti->start_ordpos(), analyzer::BindContent, ti->name(), value));
			DEBUG_EVENT5( "result", "[%d %d %d] %s '%s'", rt.back().seg(), rt.back().ofs(), rt.back().len(), rt.back().type().c_str(), rt.back().value().c_str());
		}
	}
	DEBUG_CLOSE()
	return rt;
}

void PostProcPatternMatchContext::clear()
{
	m_matcher->reset();
	m_input.clear();
}


PreProcPatternMatchContextMap::PreProcPatternMatchContextMap( const PreProcPatternMatchConfigMap& config, ErrorBufferInterface* errorhnd_)
	:m_errorhnd(errorhnd_)
	,m_debugtrace(0)
{
	DebugTraceInterface* dbgi = m_errorhnd->debugTrace();
	if (dbgi) m_debugtrace = dbgi->createTraceContext( STRUS_DBGTRACE_COMPONENT_NAME);

	PreProcPatternMatchConfigMap::const_iterator ci = config.begin(), ce = config.end();
	for (; ci != ce; ++ci)
	{
		m_ar.push_back( PreProcPatternMatchContext( *ci, errorhnd_));
	}
}

PostProcPatternMatchContextMap::PostProcPatternMatchContextMap( const PostProcPatternMatchConfigMap& config, ErrorBufferInterface* errorhnd_)
	:m_errorhnd(errorhnd_)
	,m_debugtrace(0)
{
	DebugTraceInterface* dbgi = m_errorhnd->debugTrace();
	if (dbgi) m_debugtrace = dbgi->createTraceContext( STRUS_DBGTRACE_COMPONENT_NAME);

	PostProcPatternMatchConfigMap::const_iterator ci = config.begin(), ce = config.end();
	for (; ci != ce; ++ci)
	{
		m_ar.push_back( PostProcPatternMatchContext( *ci, errorhnd_));
	}
}

void PreProcPatternMatchContextMap::clear()
{
	PreProcPatternMatchContextMap::iterator ci = begin(), ce = end();
	for (; ci != ce; ++ci)
	{
		ci->clear();
	}
}

void PostProcPatternMatchContextMap::clear()
{
	PostProcPatternMatchContextMap::iterator ci = begin(), ce = end();
	for (; ci != ce; ++ci)
	{
		ci->clear();
	}
}


