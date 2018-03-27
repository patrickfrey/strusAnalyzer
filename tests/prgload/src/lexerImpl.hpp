/*
 * Copyright (c) 2018 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Test implementation of the lexer interface for pattern matching
/// \file "lexerImpl.hpp"
#ifndef _STRUS_ANALYZER_TEST_LEXER_IMPL_HPP_INCLUDED
#define _STRUS_ANALYZER_TEST_LEXER_IMPL_HPP_INCLUDED
#include "strus/patternLexerInterface.hpp"
#include "strus/patternLexerInstanceInterface.hpp"
#include "strus/patternLexerContextInterface.hpp"
#include "strus/errorBufferInterface.hpp"
#include "strus/debugTraceInterface.hpp"
#include "strus/base/regex.hpp"
#include "strus/base/dynamic_bitset.hpp"
#include <stdexcept>
#include <map>
#include <set>
#include <algorithm>

#define STRUS_COMPONENT_NAME "lexer"

/// \brief Test implementation of the lexer interface for pattern matching
class TestPatternLexer
	:public strus::PatternLexerInterface
{
public:
	TestPatternLexer( strus::ErrorBufferInterface* errorhnd_)
		:m_errorhnd(errorhnd_){}
	virtual ~TestPatternLexer(){}

	virtual std::vector<std::string> getCompileOptionNames() const	{return std::vector<std::string>();}
	virtual strus::PatternLexerInstanceInterface* createInstance() const;
	virtual const char* getDescription() const			{return "test pattern lexer";}

private:
	strus::ErrorBufferInterface* m_errorhnd;
};


/// \brief Test implementation of the lexer instance interface for pattern matching
class TestPatternLexerInstance
	:public strus::PatternLexerInstanceInterface
{
public:
	TestPatternLexerInstance( strus::ErrorBufferInterface* errorhnd_)
		:m_errorhnd(errorhnd_),m_dbgtrace(0),m_lexemNameMap(),m_symmap(),m_expressions(),m_done(false)
	{
		strus::DebugTraceInterface* dt = m_errorhnd->debugTrace();
		m_dbgtrace = dt ? dt->createTraceContext( STRUS_COMPONENT_NAME) : NULL;
	}

	virtual ~TestPatternLexerInstance()
	{
		std::vector<Expression>::const_iterator ei = m_expressions.begin(), ee = m_expressions.end();
		for (; ei != ee; ++ei)
		{
			delete ei->regex;
		}
	}

	virtual void defineOption( const std::string& name, double value)
	{
		if (m_dbgtrace) m_dbgtrace->event( "option", "%s %f", name.c_str(), value);
		if (m_done) throw std::logic_error("illegal call");
		throw std::runtime_error("unknonw option passed to pattern lexer");
	}

	virtual void defineLexemName( unsigned int id, const std::string& name)
	{
		if (m_dbgtrace) m_dbgtrace->event( "name", "%s %u", name.c_str(), id);
		if (m_done) throw std::logic_error("illegal call");
		m_lexemNameMap[ id] = name;
	}

	virtual void defineLexem(
			unsigned int id,
			const std::string& expression,
			unsigned int resultIndex,
			unsigned int level,
			strus::analyzer::PositionBind posbind)
	{
		const char* posbindstr = NULL;
		switch (posbind)
		{
			case strus::analyzer::BindSuccessor: posbindstr = "succ"; break;
			case strus::analyzer::BindPredecessor: posbindstr = "pred"; break;
			case strus::analyzer::BindContent: posbindstr = "content"; break;
			case strus::analyzer::BindUnique: posbindstr = "unique"; break;
		}
		if (m_dbgtrace) m_dbgtrace->event( "lexem", "%u '%s' [%u] %u %s", id, expression.c_str(), resultIndex, level, posbindstr);
		if (m_done) throw std::logic_error("illegal call");
		m_expressions.push_back( Expression( id, level, new strus::RegexSearch( expression, resultIndex, m_errorhnd), posbind));
	}

	virtual void defineSymbol(
			unsigned int id,
			unsigned int lexemid,
			const std::string& name)
	{
		if (m_dbgtrace) m_dbgtrace->event( "symbol", "%u %u '%s'", id, lexemid, name.c_str());
		if (m_done) throw std::logic_error("illegal call");
		m_symmap[ lexemid][ name] = id;
	}

	virtual unsigned int getSymbol(
			unsigned int lexemid,
			const std::string& name) const
	{
		std::map<unsigned int,SymbolTable>::const_iterator si = m_symmap.find( lexemid);
		if (si == m_symmap.end()) return 0;
		SymbolTable::const_iterator ti = si->second.find( name);
		if (ti == si->second.end()) return 0;
		return ti->second;
	}

	virtual const char* getLexemName( unsigned int id) const
	{
		std::map<unsigned int,std::string>::const_iterator li = m_lexemNameMap.find( id);
		return li == m_lexemNameMap.end() ? NULL : li->second.c_str();
	}

	virtual bool compile()
	{
		if (m_done) throw std::logic_error("illegal call");
		m_done = true;
		return true;
	}

	virtual strus::PatternLexerContextInterface* createContext() const;

private:
	typedef std::map<std::string,int> SymbolTable;

	struct Expression
	{
		unsigned int id;
		unsigned int level;
		strus::RegexSearch* regex;
		strus::analyzer::PositionBind posbind;

		Expression( unsigned int id_, unsigned int level_, strus::RegexSearch* regex_, strus::analyzer::PositionBind posbind_)
			:id(id_),level(level_),regex(regex_),posbind(posbind_){}
		Expression( const Expression& o)
			:id(o.id),level(o.level),regex(o.regex),posbind(o.posbind){}
	};

	friend class TestPatternLexerContext;

	strus::ErrorBufferInterface* m_errorhnd;
	strus::DebugTraceContextInterface* m_dbgtrace;
	std::map<unsigned int,std::string> m_lexemNameMap;
	std::map<unsigned int,SymbolTable> m_symmap;
	std::vector<Expression> m_expressions;
	bool m_done;
};

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

/// \brief Test implementation of the lexer context interface for pattern matching
class TestPatternLexerContext
	:public strus::PatternLexerContextInterface
{
public:
	TestPatternLexerContext( strus::ErrorBufferInterface* errorhnd_, const TestPatternLexerInstance* instance_)
		:m_errorhnd(errorhnd_),m_dbgtrace(0),m_instance(instance_)
	{
		strus::DebugTraceInterface* dt = m_errorhnd->debugTrace();
		m_dbgtrace = dt ? dt->createTraceContext( STRUS_COMPONENT_NAME) : NULL;
	}
	virtual ~TestPatternLexerContext(){}

	virtual std::vector<strus::analyzer::PatternLexem> match( const char* src, std::size_t srclen)
	{
		typedef TestPatternLexerInstance::Expression Expression;
		if (m_dbgtrace)
		{
			std::string segstr( contentCut( src, srclen, 100));
			m_dbgtrace->event( "segment", "[%s] %u", segstr.c_str(), (unsigned int)srclen);
			m_dbgtrace->open( "match");
		}
		// Get matches for every lexem:
		std::vector<Match> matchar;
		std::vector<Expression>::const_iterator ri = m_instance->m_expressions.begin(), re = m_instance->m_expressions.end();
		for (int ridx=0; ri != re; ++ri)
		{
			char const* si = src;
			const char* se = src + srclen;
			for (; si < se; ++si)
			{
				strus::RegexSearch::Match rxmatch = ri->regex->find( si, se);
				if (!rxmatch.valid()) break;
				// ... no match found anymore, break
				if (!matchar.empty() && (matchar.back().pos + matchar.back().len >= rxmatch.pos + rxmatch.len)) continue;
				// ... do not include matches covered by previous matches
				matchar.push_back( Match( ridx, ri->level, rxmatch.pos, rxmatch.len));
			}
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
				strus::analyzer::PositionBind posbind = m_instance->m_expressions[ mi->idx].posbind;
				switch (posbind)
				{
					case strus::analyzer::BindSuccessor:
					case strus::analyzer::BindPredecessor:
						break;
					case strus::analyzer::BindContent:
						positions.insert( mi->pos);
						break;
					case strus::analyzer::BindUnique:
						for (lastpos=mi->pos,++mi,++midx; mi != me; ++mi)
						{
							if (!elimset.test( midx))
							{
								posbind = m_instance->m_expressions[ mi->idx].posbind;
								if (posbind == strus::analyzer::BindContent) break;
								if (posbind == strus::analyzer::BindUnique)
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
		std::vector<strus::analyzer::PatternLexem> rt;
		mi = matchar.begin(), me = matchar.end();
		for (int midx=0; mi != me; ++mi,++midx)
		{
			if (!elimset.test( midx))
			{
				strus::analyzer::PositionBind posbind = m_instance->m_expressions[ mi->idx].posbind;
				int ordpos = 1;
				std::map<int,int>::const_iterator pi;
				switch (posbind)
				{
					case strus::analyzer::BindSuccessor:
					case strus::analyzer::BindUnique:
						pi = ordposmap.upper_bound( mi->pos-1);
						if (pi == ordposmap.end()) continue;
						ordpos = pi->second;
						break;
					case strus::analyzer::BindPredecessor:
						pi = ordposmap.upper_bound( mi->pos);
						if (pi == ordposmap.begin()) continue;
						--pi;
						ordpos = pi->second;
						break;
					case strus::analyzer::BindContent:
						pi = ordposmap.find( mi->pos);
						if (pi == ordposmap.end()) throw std::runtime_error("internal error in position assignment");
						ordpos = pi->second;
						break;
				}
				int id = m_instance->m_expressions[ mi->idx].id;
				if (m_instance->m_symmap.find( id) != m_instance->m_symmap.end())
				{
					std::string symkey( src+mi->pos, mi->len);
					int symid = m_instance->getSymbol( id, symkey);
					if (symid) id = symid;
				}
				rt.push_back( strus::analyzer::PatternLexem( id, ordpos, 0, mi->pos, mi->len));
			}
		}
		if (m_dbgtrace)
		{
			std::vector<strus::analyzer::PatternLexem>::const_iterator xi = rt.begin(), xe = rt.end();
			for (; xi != xe; ++xi)
			{
				std::string elem( src+xi->origpos(), xi->origsize());
				m_dbgtrace->event( "result", "%u %u:%d '%s'", xi->ordpos(), xi->origpos(), xi->origsize(), elem.c_str());
			}
			m_dbgtrace->close();
		}
		return rt;
	}

	virtual void reset()
	{
	}

private:
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

private:
	strus::ErrorBufferInterface* m_errorhnd;
	strus::DebugTraceContextInterface* m_dbgtrace;
	const TestPatternLexerInstance* m_instance;
};



strus::PatternLexerContextInterface* TestPatternLexerInstance::createContext() const
{
	return new TestPatternLexerContext( m_errorhnd, this);
}

strus::PatternLexerInstanceInterface* TestPatternLexer::createInstance() const
{
	return new TestPatternLexerInstance( m_errorhnd);
}

#endif

