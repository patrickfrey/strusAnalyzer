/*
 * Copyright (c) 2018 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Test implementation of the lexer interface for pattern matching
/// \file "patternLexer.cpp"
#include "patternLexer.hpp"
#include "strus/errorBufferInterface.hpp"
#include "strus/debugTraceInterface.hpp"
#include "strus/analyzer/patternLexem.hpp"
#include "strus/base/dynamic_bitset.hpp"
#include "strus/base/string_format.hpp"
#include "private/internationalization.hpp"
#include "private/errorUtils.hpp"
#include <stdexcept>
#include <map>
#include <set>
#include <algorithm>

using namespace strus;

#define STRUS_DBGTRACE_COMPONENT_NAME "lexer"
#define STRUS_DBGTRACE_COMPONENT_NAME_PROC "lexer_proc"

PatternLexerInstanceInterface* TestPatternLexer::createInstance() const
{
	return new TestPatternLexerInstance( m_errorhnd);
}


TestPatternLexerInstance::TestPatternLexerInstance( ErrorBufferInterface* errorhnd_)
	:m_errorhnd(errorhnd_),m_dbgtrace(0),m_lexemNameMap(),m_symmap(),m_expressions(),m_done(false)
{
	DebugTraceInterface* dt = m_errorhnd->debugTrace();
	m_dbgtrace = dt ? dt->createTraceContext( STRUS_DBGTRACE_COMPONENT_NAME) : NULL;
}

TestPatternLexerInstance::~TestPatternLexerInstance()
{
	std::vector<Expression>::const_iterator ei = m_expressions.begin(), ee = m_expressions.end();
	for (; ei != ee; ++ei)
	{
		delete ei->regex;
	}
	if (m_dbgtrace) delete m_dbgtrace;
}

void TestPatternLexerInstance::defineOption( const std::string& name, double value)
{
	try
	{
		if (m_dbgtrace) m_dbgtrace->event( "option", "%s %f", name.c_str(), value);
		if (m_done) throw std::runtime_error( _TXT("illegal call"));
		throw std::runtime_error(_TXT("unknonw option passed to pattern lexer"));
	}
	CATCH_ERROR_ARG1_MAP( _TXT("error calling %s: %s"), "TestPatternLexerInstance::defineOption", *m_errorhnd);
}

void TestPatternLexerInstance::defineLexemName( unsigned int id, const std::string& name)
{
	try
	{
		if (m_dbgtrace) m_dbgtrace->event( "name", "%s %u", name.c_str(), id);
		if (m_done) throw std::runtime_error( _TXT("illegal call"));
		m_lexemNameMap[ id] = name;
	}
	CATCH_ERROR_ARG1_MAP( _TXT("error calling %s: %s"), "TestPatternLexerInstance::defineOption", *m_errorhnd);
}

void TestPatternLexerInstance::defineLexem(
		unsigned int id,
		const std::string& expression,
		unsigned int resultIndex,
		unsigned int level,
		analyzer::PositionBind posbind)
{
	try
	{
		const char* posbindstr = NULL;
		switch (posbind)
		{
			case analyzer::BindSuccessor: posbindstr = "succ"; break;
			case analyzer::BindPredecessor: posbindstr = "pred"; break;
			case analyzer::BindContent: posbindstr = "content"; break;
			case analyzer::BindUnique: posbindstr = "unique"; break;
		}
		if (m_dbgtrace) m_dbgtrace->event( "lexem", "%u '%s' [%u] %u %s", id, expression.c_str(), resultIndex, level, posbindstr);
		if (m_done) throw std::runtime_error( _TXT("illegal call"));
		m_expressions.push_back( Expression( id, level, new strus::RegexSearch( expression, resultIndex, m_errorhnd), posbind));
	}
	CATCH_ERROR_ARG1_MAP( _TXT("error calling %s: %s"), "TestPatternLexerInstance::defineOption", *m_errorhnd);
}

void TestPatternLexerInstance::defineSymbol(
		unsigned int id,
		unsigned int lexemid,
		const std::string& name)
{
	try
	{
		if (m_dbgtrace) m_dbgtrace->event( "symbol", "%u %u '%s'", id, lexemid, name.c_str());
		if (m_done) throw std::runtime_error( _TXT("illegal call"));
		m_symmap[ lexemid][ name] = id;
	}
	CATCH_ERROR_ARG1_MAP( _TXT("error calling %s: %s"), "TestPatternLexerInstance::defineOption", *m_errorhnd);
}

unsigned int TestPatternLexerInstance::getSymbol(
		unsigned int lexemid,
		const std::string& name) const
{
	try
	{
		std::map<unsigned int,SymbolTable>::const_iterator si = m_symmap.find( lexemid);
		if (si == m_symmap.end()) return 0;
		SymbolTable::const_iterator ti = si->second.find( name);
		if (ti == si->second.end()) return 0;
		return ti->second;
	}
	CATCH_ERROR_ARG1_MAP_RETURN( _TXT("error calling %s: %s"), "TestPatternLexerInstance::defineOption", *m_errorhnd, 0);
}

const char* TestPatternLexerInstance::getLexemName( unsigned int id) const
{
	std::map<unsigned int,std::string>::const_iterator li = m_lexemNameMap.find( id);
	return li == m_lexemNameMap.end() ? NULL : li->second.c_str();
}

bool TestPatternLexerInstance::compile()
{
	try
	{
		if (m_dbgtrace) m_dbgtrace->event( "compile", "lexems %d named %d symbols %d",
							(int)m_expressions.size(), (int)m_lexemNameMap.size(), (int)m_symmap.size());
		if (m_done) throw std::runtime_error( _TXT("illegal call"));
		m_done = true;
		return true;
	}
	CATCH_ERROR_ARG1_MAP_RETURN( _TXT("error calling %s: %s"), "TestPatternLexerInstance::defineOption", *m_errorhnd, false);
}

PatternLexerContextInterface* TestPatternLexerInstance::createContext() const
{
	try
	{
		return new TestPatternLexerContext( m_errorhnd, this);
	}
	CATCH_ERROR_ARG1_MAP_RETURN( _TXT("error calling %s: %s"), "TestPatternLexerInstance::defineOption", *m_errorhnd, NULL);
}


// Match candidate structure
struct Match
{
	int idx;
	int level;
	int pos;
	int len;

	Match( int idx_, int level_, int pos_, int len_)
		:idx(idx_),level(level_),pos(pos_),len(len_){}
	Match( const Match& o)
		:idx(o.idx),level(o.level),pos(o.pos),len(o.len){}

	bool operator < (const Match& o) const
	{
		if (pos < o.pos) return true;
		if (pos > o.pos) return false;
		if (level < o.level) return true;
		if (level > o.level) return false;
		if (len > o.len) return true;
		if (len < o.len) return false;
		if (idx < o.idx) return true;
		if (idx > o.idx) return false;
		return false;
	}
};

static std::string contentCut( const char* str, std::size_t size, std::size_t len)
{
	enum {B11000000=192,B10000000=128};
	if (len >= size) len = size;
	while (len && (str[ len-1] & B11000000) == B10000000) --len;
	std::string rt;
	for (std::size_t si=0; si<len; ++si)
	{
		if ((unsigned char)str[si] < 32) rt.push_back('_'); else rt.push_back(str[si]);
	}
	return rt;
}

TestPatternLexerContext::TestPatternLexerContext( ErrorBufferInterface* errorhnd_, const TestPatternLexerInstance* instance_)
	:m_errorhnd(errorhnd_),m_dbgtrace(0),m_dbgtrace_proc(0),m_instance(instance_)
{
	DebugTraceInterface* dt = m_errorhnd->debugTrace();
	m_dbgtrace = dt ? dt->createTraceContext( STRUS_DBGTRACE_COMPONENT_NAME) : NULL;
	m_dbgtrace_proc = dt ? dt->createTraceContext( STRUS_DBGTRACE_COMPONENT_NAME_PROC) : NULL;
}
TestPatternLexerContext::~TestPatternLexerContext()
{
	if (m_dbgtrace) delete m_dbgtrace;
	if (m_dbgtrace_proc) delete m_dbgtrace_proc;
}

std::vector<analyzer::PatternLexem> TestPatternLexerContext::match( const char* src, std::size_t srclen)
{
	try
	{
		typedef TestPatternLexerInstance::Expression Expression;
		if (m_dbgtrace || m_dbgtrace_proc)
		{
			std::string segstr( contentCut( src, srclen, 100));
			if (m_dbgtrace)
			{
				m_dbgtrace->event( "input", "[%s] %u", segstr.c_str(), (unsigned int)srclen);
			}
			else
			{
				m_dbgtrace_proc->event( "input", "[%s] %u", segstr.c_str(), (unsigned int)srclen);
			}
		}
		// Get matches for every lexem:
		std::vector<Match> matchar;
		std::size_t matcharpos = 0;
		std::vector<Expression>::const_iterator ri = m_instance->m_expressions.begin(), re = m_instance->m_expressions.end();
		for (int ridx=0; ri != re; ++ri,++ridx)
		{
			if (m_dbgtrace_proc)
			{
				std::map<unsigned int,std::string>::const_iterator li = m_instance->m_lexemNameMap.find( ri->id);
				m_dbgtrace_proc->open( "expression", li == m_instance->m_lexemNameMap.end() ? strus::string_format("%d",ri->id):li->second);
			}
			char const* si = src;
			const char* se = src + srclen;
			for (; si < se; ++si)
			{
				strus::RegexSearch::Match rxmatch = ri->regex->find( si, se);
				if (!rxmatch.valid()) break;
				rxmatch.pos += si - src;
				// ... no match found anymore, break
				if (matchar.size() > matcharpos && (matchar.back().pos + matchar.back().len >= rxmatch.pos + rxmatch.len)) continue;
				// ... do not include matches covered by previous matches
				si = src + rxmatch.pos;
				matchar.push_back( Match( ridx, ri->level, rxmatch.pos, rxmatch.len));
			}
			if (m_dbgtrace_proc)
			{
				std::vector<Match>::const_iterator mi = matchar.begin()+matcharpos, me = matchar.end();
				for (; mi != me; ++mi)
				{
					std::string content = contentCut( src + mi->pos, mi->len, 40);
					m_dbgtrace_proc->event( "candidate", "%d %d %s", mi->pos, mi->len, content.c_str());
				}
				m_dbgtrace_proc->close();
			}
			matcharpos = matchar.size();
		}
		// Sort matches and eliminate elements covered by elements with a higher level:
		std::sort( matchar.begin(), matchar.end());
		strus::dynamic_bitset elimset( matchar.size());
		std::vector<Match>::const_iterator mi = matchar.begin(), me = matchar.end();
		for (int midx=0; mi != me; ++mi,++midx)
		{
			std::vector<Match>::const_iterator ni = mi, ne = me;
			int nidx = midx;
			for (++ni,++nidx; ni != ne; ++ni,++nidx)
			{
				if (ni->pos + ni->len > mi->pos + mi->len) break;
				if (mi->level > ni->level) elimset.set( nidx);
			}
		}
		// Assign ordinal positions to positions:
		std::set<int> positions;
		mi = matchar.begin(), me = matchar.end();
		for (int midx=0; mi != me; ++mi,++midx)
		{
			if (!elimset.test( midx))
			{
				int lastpos;
				analyzer::PositionBind posbind = m_instance->m_expressions[ mi->idx].posbind;
				switch (posbind)
				{
					case analyzer::BindSuccessor:
					case analyzer::BindPredecessor:
						break;
					case analyzer::BindContent:
						positions.insert( mi->pos);
						break;
					case analyzer::BindUnique:
						for (lastpos=mi->pos,++mi,++midx; mi != me; ++mi)
						{
							if (!elimset.test( midx))
							{
								posbind = m_instance->m_expressions[ mi->idx].posbind;
								if (posbind == analyzer::BindContent) break;
								if (posbind == analyzer::BindUnique)
								{
									lastpos = mi->pos;
								}
							}
						}
						--mi,--midx; //... compensate for loop increment
						positions.insert( lastpos);
						break;
				}
			}
		}
		std::map<int,int> ordposmap;
		{
			std::set<int>::const_iterator pi = positions.begin(), pe = positions.end();
			for (int pidx=0; pi != pe; ++pi)
			{
				ordposmap[ *pi] = ++pidx;
			}
		}
		// Build result:
		std::vector<analyzer::PatternLexem> rt;
		mi = matchar.begin(), me = matchar.end();
		for (int midx=0; mi != me; ++mi,++midx)
		{
			if (!elimset.test( midx))
			{
				analyzer::PositionBind posbind = m_instance->m_expressions[ mi->idx].posbind;
				int ordpos = 1;
				std::map<int,int>::const_iterator pi;
				switch (posbind)
				{
					case analyzer::BindSuccessor:
					case analyzer::BindUnique:
						pi = ordposmap.upper_bound( mi->pos-1);
						if (pi == ordposmap.end()) continue;
						ordpos = pi->second;
						break;
					case analyzer::BindPredecessor:
						pi = ordposmap.upper_bound( mi->pos);
						if (pi == ordposmap.begin()) continue;
						--pi;
						ordpos = pi->second;
						break;
					case analyzer::BindContent:
						pi = ordposmap.find( mi->pos);
						if (pi == ordposmap.end()) throw std::runtime_error(_TXT("internal error in position assignment"));
						ordpos = pi->second;
						break;
				}
				int id = m_instance->m_expressions[ mi->idx].id;
				if (m_instance->m_symmap.find( id) != m_instance->m_symmap.end())
				{
					std::string symkey( src+mi->pos, mi->len);
					int symid = m_instance->getSymbol( id, symkey);
					if (symid)
					{
						rt.push_back( analyzer::PatternLexem( symid, ordpos, 0, mi->pos, mi->len));
					}
				}
				rt.push_back( analyzer::PatternLexem( id, ordpos, 0, mi->pos, mi->len));
			}
		}
		if (m_dbgtrace)
		{
			std::vector<analyzer::PatternLexem>::const_iterator xi = rt.begin(), xe = rt.end();
			for (; xi != xe; ++xi)
			{
				std::string elem( src+xi->origpos(), xi->origsize());
				std::map<unsigned int,std::string>::const_iterator li = m_instance->m_lexemNameMap.find( xi->id());
				std::string elemname( li == m_instance->m_lexemNameMap.end() ? strus::string_format("%d",xi->id()) : li->second);
				m_dbgtrace->event( "match", "%u %u:%d %s '%s'", xi->ordpos(), xi->origpos(), xi->origsize(), elemname.c_str(), elem.c_str());
			}
		}
		return rt;
	}
	CATCH_ERROR_ARG1_MAP_RETURN( _TXT("error calling %s: %s"), "TestPatternLexerInstance::defineOption", *m_errorhnd, std::vector<analyzer::PatternLexem>());
}

void TestPatternLexerContext::reset()
{
}


