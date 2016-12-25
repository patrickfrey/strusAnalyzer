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
#include <cstring>
#include <algorithm>
/*[-]*/#include <iostream>

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
			rt.push_back( BindTerm( ri->start_origseg(), ri->start_origpos(), m_config->patternTypeName(), ri->name(), analyzer::BindContent));
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
				rt.push_back( BindTerm( ti->start_origseg(), ti->start_origpos(), ti->name(), std::string( chunk, chunksize), analyzer::BindContent));
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
				rt.push_back( BindTerm( ti->start_origseg(), ti->start_origpos(), ti->name(), value, analyzer::BindContent));
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

	// Convert input elements:
	std::vector<analyzer::PatternLexem> pinput;
	std::vector<BindTerm>::const_iterator bi = m_input.begin(), be = m_input.end();
	for (std::size_t bidx=0; bi != be; ++bi,++bidx)
	{
		unsigned int lexemid = m_feeder->getLexem( bi->type());
		unsigned int symbolid = m_feeder->getSymbol( lexemid, bi->value());
		if (symbolid)
		{
			pinput.push_back( analyzer::PatternLexem( symbolid, 0, 0/*seg*/, bidx/*ofs*/, 1));
		}
		else
		{
			pinput.push_back( analyzer::PatternLexem( lexemid, 0, 0/*seg*/, bidx/*ofs*/, 1));
		}
	}

	// Assign ordinal position:
	std::vector<analyzer::PatternLexem>::iterator pi = pinput.begin(), pe = pinput.end();
	std::size_t ordpos = 0;
	while (pi != pe)
	{
		std::vector<analyzer::PatternLexem>::iterator pa = pi;
		++ordpos;
		for (; pi != pe && pi->origpos() == pa->origpos() && pi->origseg() == pa->origseg(); ++pi)
		{
			pi->setOrdpos( ordpos);
		}
	}

	// Feed input to matcher:
	pi = pinput.begin(), pe = pinput.end();
	for (; pi != pe; ++pi)
	{
		/*[-]*/std::cout << "INPUT " << pi->id() << " pos " << pi->ordpos() << " orig " << pi->origseg() << ":" << pi->origpos() << " size " << pi->origsize() << std::endl;
		m_matcher->putInput( *pi);
	}
	/*[-]*/std::cout << "DONE" << std::endl;

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
			rt.push_back( BindTerm( startelem.seg(), startelem.ofs(), m_config->patternTypeName(), ri->name(), analyzer::BindContent));
		}
		std::vector<analyzer::PatternMatcherResultItem>::const_iterator ti = ri->items().begin(), te = ri->items().end();
		for (; ti != te; ++ti)
		{
			const BindTerm& startelem = m_input[ ti->start_origpos()];
			if (ti->end_origpos() == ti->start_origpos() +1)
			{
				rt.push_back( BindTerm( startelem.seg(), startelem.ofs(), ti->name(), startelem.value(), analyzer::BindContent));
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
				rt.push_back( BindTerm( startelem.seg(), startelem.ofs(), ti->name(), value, analyzer::BindContent));
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


