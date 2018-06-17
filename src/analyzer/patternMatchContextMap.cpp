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
#include "strus/lib/pattern_resultformat.hpp"
#include "private/debugTraceHelpers.hpp"
#include <cstring>
#include <algorithm>
#include <limits>
#include <iostream>
#include <sstream>
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
	,m_content()
	,m_segPosContentPosMap()
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

PreProcPatternMatchContext::~PreProcPatternMatchContext()
{
	if (m_debugtrace) delete m_debugtrace;
}

void PreProcPatternMatchContext::process( const SegmenterPosition& segpos, const char* seg, std::size_t segsize)
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
		li->origpos().setSeg( segpos);
		m_matcher->putInput( *li);
		DEBUG_EVENT5( "lexem", "%d pos %d [%d %d %d]", li->id(), li->ordpos(), li->origpos().seg(), li->origpos().ofs(), li->origsize());
	}
	DEBUG_CLOSE()
}

std::string PreProcPatternMatchContext::formatResult( const char* value) const
{
	std::ostringstream out;
	strus::PatternResultFormatChunk chunk;
	char const* vi = value;
	while (strus::PatternResultFormatChunk::parseNext( chunk, vi))
	{
		if (chunk.value)
		{
			out << std::string( chunk.value, chunk.valuesize);
		}
		else
		{
			SegPosContentPosMap::const_iterator starti = m_segPosContentPosMap.find( chunk.start_seg);
			SegPosContentPosMap::const_iterator endi = m_segPosContentPosMap.find( chunk.end_seg);
			if (starti == m_segPosContentPosMap.end() || endi == m_segPosContentPosMap.end()) throw std::runtime_error(_TXT("corrupt result segment position"));

			std::size_t start_srcpos = starti->second + chunk.start_pos;
			std::size_t end_srcpos = endi->second + chunk.end_pos;
			std::size_t len = end_srcpos - start_srcpos;
			out << std::string( m_content.c_str()+start_srcpos, len);
		}
	}
	return out.str();
}

std::vector<BindTerm> PreProcPatternMatchContext::fetchResults()
{
	std::vector<BindTerm> rt;
	std::vector<analyzer::PatternMatcherResult> results = m_matcher->fetchResults();
	std::vector<analyzer::PatternMatcherResult>::const_iterator ri = results.begin(), re = results.end();
	DEBUG_OPEN( "result")
	for (; ri != re; ++ri)
	{
		if (!m_config->allowCrossSegmentMatches() && ri->origpos().seg() != ri->origend().seg()) continue;
		if (ri->value())
		{
			rt.push_back( BindTerm( ri->origpos().seg(), ri->origpos().ofs(), ri->ordend() - ri->ordpos(), analyzer::BindContent, m_config->patternTypeName(), formatResult( ri->value())));
		}
		else
		{
			rt.push_back( BindTerm( ri->origpos().seg(), ri->origpos().ofs(), ri->ordend() - ri->ordpos(), analyzer::BindContent, m_config->patternTypeName(), ri->name()));
		}
		DEBUG_EVENT5( "result", "[%d %d %d] %s '%s'", rt.back().seg(), rt.back().ofs(), rt.back().len(), rt.back().type().c_str(), rt.back().value().c_str());
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

PostProcPatternMatchContext::~PostProcPatternMatchContext()
{
	if (m_debugtrace) delete m_debugtrace;
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

std::string PostProcPatternMatchContext::formatResult( const char* value) const
{
	std::ostringstream out;
	strus::PatternResultFormatChunk chunk;
	char const* vi = value;
	while (strus::PatternResultFormatChunk::parseNext( chunk, vi))
	{
		if (chunk.value)
		{
			out << std::string( chunk.value, chunk.valuesize);
		}
		else
		{
			int idx = chunk.start_pos;
			for (;idx < chunk.end_pos; ++idx)
			{
				const BindLexem& lexem = m_input[ idx];
				if (idx == chunk.start_pos) out << ' ';
				out << lexem.value();
			}
		}
	}
	return out.str();
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
	{
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
			pinput.push_back( analyzer::PatternLexem( bi->id(), ordpos, analyzer::Position(bi->seg(), virtpos), 1));
		}
	}
	// Feed input to matcher:
	DEBUG_OPEN( "input")
	{
		std::vector<analyzer::PatternLexem>::const_iterator pi = pinput.begin(), pe = pinput.end();
		for (; pi != pe; ++pi)
		{
			if (pi->id())
			{
				int virtpos = pi->origpos().ofs();
				int origpos = m_input[ virtpos].ofs();
				DEBUG_EVENT5( "lexem", "%d pos %d [%d %d %d]", pi->id(), pi->ordpos(), pi->origpos().seg(), origpos, pi->origsize());
				m_matcher->putInput( *pi);
			}
		}
	}
	DEBUG_CLOSE()

	// Build result:
	std::vector<BindTerm> rt;
	std::vector<analyzer::PatternMatcherResult> results = m_matcher->fetchResults();
	std::vector<analyzer::PatternMatcherResult>::const_iterator ri = results.begin(), re = results.end();
	for (; ri != re; ++ri)
	{
		if (!m_config->allowCrossSegmentMatches() && ri->origpos().seg() != ri->origend().seg()) continue;
		int start_virtpos = ri->origpos().ofs();
		int start_origpos = m_input[ start_virtpos].ofs();

		if (ri->value())
		{
			rt.push_back( BindTerm( ri->origpos().seg(), start_origpos, ri->ordend() - ri->ordpos(), analyzer::BindContent, m_config->patternTypeName(), formatResult( ri->value())));
		}
		else
		{
			rt.push_back( BindTerm( ri->origpos().seg(), start_origpos, ri->ordend() - ri->ordpos(), analyzer::BindContent, m_config->patternTypeName(), ri->name()));
		}
		DEBUG_EVENT5( "result", "[%d %d %d] %s '%s'", rt.back().seg(), rt.back().ofs(), rt.back().len(), rt.back().type().c_str(), rt.back().value().c_str());
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
		m_ar.push_back( new PreProcPatternMatchContext( *ci, errorhnd_));
	}
}

PreProcPatternMatchContextMap::~PreProcPatternMatchContextMap()
{
	if (m_debugtrace) delete m_debugtrace;
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
		m_ar.push_back( new PostProcPatternMatchContext( *ci, errorhnd_));
	}
}

PostProcPatternMatchContextMap::~PostProcPatternMatchContextMap()
{
	if (m_debugtrace) delete m_debugtrace;
}

void PreProcPatternMatchContextMap::clear()
{
	PreProcPatternMatchContextMap::iterator ci = begin(), ce = end();
	for (; ci != ce; ++ci)
	{
		(*ci)->clear();
	}
}

void PostProcPatternMatchContextMap::clear()
{
	PostProcPatternMatchContextMap::iterator ci = begin(), ce = end();
	for (; ci != ce; ++ci)
	{
		(*ci)->clear();
	}
}


