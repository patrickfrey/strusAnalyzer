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
#include <cstring>
#include <algorithm>
#include <limits>

using namespace strus;

PreProcPatternMatchContext::PreProcPatternMatchContext( const PreProcPatternMatchConfig& config)
	:m_config(&config)
	,m_matcher(config.matcher()->createContext())
	,m_lexer(config.lexer()->createContext())
{
	if (!m_matcher.get())
	{
		throw strus::runtime_error( _TXT("failed to create pattern matcher context"));
	}
	if (!m_lexer.get())
	{
		throw strus::runtime_error( _TXT("failed to create pattern lexer context"));
	}
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
	std::vector<analyzer::PatternLexem> lexems = m_lexer->match( seg, segsize);
	std::vector<analyzer::PatternLexem>::iterator li = lexems.begin(), le = lexems.end();
	for (; li != le; ++li)
	{
		li->setOrigseg( segpos);
		m_matcher->putInput( *li);
	}
}

std::vector<BindTerm> PreProcPatternMatchContext::fetchResults()
{
	std::vector<BindTerm> rt;
	std::vector<analyzer::PatternMatcherResult> results = m_matcher->fetchResults();
	std::vector<analyzer::PatternMatcherResult>::const_iterator ri = results.begin(), re = results.end();
	for (; ri != re; ++ri)
	{
		if (!m_config->allowCrossSegmentMatches() && ri->start_origseg() != ri->end_origseg()) continue;
		if (!m_config->patternTypeName().empty())
		{
			rt.push_back( BindTerm( ri->start_origseg(), ri->start_origpos(), ri->end_ordpos() - ri->start_ordpos(), analyzer::BindContent, m_config->patternTypeName(), ri->name()));
		}
		std::vector<analyzer::PatternMatcherResultItem>::const_iterator ti = ri->items().begin(), te = ri->items().end();
		for (; ti != te; ++ti)
		{
			SegPosContentPosMap::const_iterator segi;
			segi = m_segPosContentPosMap.find( ti->start_origseg());
			if (segi == m_segPosContentPosMap.end()) throw strus::runtime_error(_TXT("internal: inconsistency in data, segment position unknown"));
			std::size_t segstritr = segi->second;
			segi = m_segPosContentPosMap.find( ti->end_origseg());
			if (segi == m_segPosContentPosMap.end()) throw strus::runtime_error(_TXT("internal: inconsistency in data, segment position unknown"));
			std::size_t segstrend = segi->second;

			if (segstritr == segstrend)
			{
				const char* chunk = m_content.c_str() + segstritr + ti->start_origpos();
				std::size_t chunksize = ti->end_origpos() - ti->start_origpos();
				rt.push_back( BindTerm( ti->start_origseg(), ti->start_origpos(), ti->end_ordpos() - ti->start_ordpos(), analyzer::BindContent, ti->name(), std::string( chunk, chunksize)));
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
					throw strus::runtime_error(_TXT("internal: inconsistency in data, segments overlapping or not in ascending order"));
				}
				value.append( m_content.c_str() + segstritr, ti->end_origpos());
				rt.push_back( BindTerm( ti->start_origseg(), ti->start_origpos(), ti->end_ordpos() - ti->start_ordpos(), analyzer::BindContent, ti->name(), value));
			}
		}
	}
	return rt;
}

void PreProcPatternMatchContext::clear()
{
	m_matcher->reset();
	m_lexer->reset();
	m_content.clear();
	m_segPosContentPosMap.clear();
}

PostProcPatternMatchContext::PostProcPatternMatchContext( const PostProcPatternMatchConfig& config)
	:m_config(&config)
	,m_matcher(config.matcher()->createContext())
	,m_feeder(config.feeder())
{
	if (!m_matcher.get())
	{
		throw strus::runtime_error( _TXT("failed to create pattern matcher context"));
	}
}

void PostProcPatternMatchContext::process( const std::vector<BindTerm>& input)
{
	std::vector<BindTerm>::const_iterator bi = input.begin(), be = input.end();
	for (; bi != be; ++bi)
	{
		unsigned int lexemid = m_feeder->getLexem( bi->type());
		if (lexemid)
		{
			m_input.push_back( *bi);
		}
	}
}

std::vector<BindTerm> PostProcPatternMatchContext::fetchResults()
{
	std::sort( m_input.begin(), m_input.end());
	unsigned int prev_seg = std::numeric_limits<unsigned int>::max();
	unsigned int prev_ofs = std::numeric_limits<unsigned int>::max();
	unsigned int ordpos = 0;

	// Convert input elements:
	std::vector<analyzer::PatternLexem> pinput;
	std::vector<BindTerm>::const_iterator bi = m_input.begin(), be = m_input.end();
	for (std::size_t bidx=0; bi != be; ++bi,++bidx)
	{
		unsigned int id = m_feeder->getLexem( bi->type());
		if (!bi->value().empty())
		{
			id = m_feeder->getSymbol( id, bi->value());
		}
		if (bi->seg() != prev_seg || bi->ofs() != prev_ofs)
		{
			// Increment ordinal position:
			++ordpos;
			prev_seg = bi->seg();
			prev_ofs = bi->ofs();
		}
		pinput.push_back( analyzer::PatternLexem( id, ordpos, 0/*seg*/, bidx/*ofs*/, 1));
	}

	// Feed input to matcher:
	std::vector<analyzer::PatternLexem>::const_iterator pi = pinput.begin(), pe = pinput.end();
	for (; pi != pe; ++pi)
	{
		if (pi->id())
		{
			m_matcher->putInput( *pi);
		}
	}

	// Build result:
	std::vector<BindTerm> rt;
	std::vector<analyzer::PatternMatcherResult> results = m_matcher->fetchResults();
	std::vector<analyzer::PatternMatcherResult>::const_iterator ri = results.begin(), re = results.end();
	for (; ri != re; ++ri)
	{
		if (!m_config->allowCrossSegmentMatches() && ri->start_origseg() != ri->end_origseg()) continue;

		if (!m_config->patternTypeName().empty())
		{
			const BindTerm& startelem = m_input[ ri->start_origpos()];
			rt.push_back( BindTerm( startelem.seg(), startelem.ofs(), ri->end_ordpos() - ri->start_ordpos(), analyzer::BindContent, m_config->patternTypeName(), ri->name()));
		}
		std::vector<analyzer::PatternMatcherResultItem>::const_iterator ti = ri->items().begin(), te = ri->items().end();
		for (; ti != te; ++ti)
		{
			const BindTerm& startelem = m_input[ ti->start_origpos()];
			if (ti->end_origpos() == ti->start_origpos() +1)
			{
				rt.push_back( BindTerm( startelem.seg(), startelem.ofs(), ti->end_ordpos() - ti->start_ordpos(), analyzer::BindContent, ti->name(), startelem.value()));
			}
			else
			{
				std::string value;
				std::size_t ei = ti->start_origpos(), ee = ti->end_origpos();
				for (std::size_t eidx=0; ei != ee; ++ei,++eidx)
				{
					if (eidx) value.push_back(' ');
					value.append( m_input[ ei].value());
				}
				rt.push_back( BindTerm( startelem.seg(), startelem.ofs(), ti->end_ordpos() - ti->start_ordpos(), analyzer::BindContent, ti->name(), value));
			}
		}
	}
	return rt;
}

void PostProcPatternMatchContext::clear()
{
	m_matcher->reset();
	m_input.clear();
}


PreProcPatternMatchContextMap::PreProcPatternMatchContextMap( const PreProcPatternMatchConfigMap& config)
{
	PreProcPatternMatchConfigMap::const_iterator ci = config.begin(), ce = config.end();
	for (; ci != ce; ++ci)
	{
		m_ar.push_back( PreProcPatternMatchContext( *ci));
	}
}

PostProcPatternMatchContextMap::PostProcPatternMatchContextMap( const PostProcPatternMatchConfigMap& config)
{
	PostProcPatternMatchConfigMap::const_iterator ci = config.begin(), ce = config.end();
	for (; ci != ce; ++ci)
	{
		m_ar.push_back( PostProcPatternMatchContext( *ci));
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


