/*
 * Copyright (c) 2018 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Test implementation of the lexer interface for pattern matching
/// \file "patternMatcher.hpp"
#include "patternMatcher.hpp"
#include "strus/errorBufferInterface.hpp"
#include "strus/debugTraceInterface.hpp"
#include "strus/analyzer/patternLexem.hpp"
#include "strus/analyzer/patternMatcherResult.hpp"
#include "private/internationalization.hpp"
#include "private/errorUtils.hpp"
#include <stdexcept>
#include <string>
#include <map>
#include <set>
#include <algorithm>
#include <limits>
#include <cstring>

using namespace strus;

#define STRUS_DBGTRACE_COMPONENT_NAME "pattern"
#define STRUS_DBGTRACE_COMPONENT_NAME_PROC "pattern_proc"

enum {
	MaxId=(1<<20),
	PatternIdOfs=(1<<28),
	ExpressionIdOfs=(1<<29)
};

PatternMatcherInstanceInterface* TestPatternMatcher::createInstance() const
{
	return new TestPatternMatcherInstance( m_errorhnd);
}

TestPatternMatcherInstance::TestPatternMatcherInstance( ErrorBufferInterface* errorhnd_)
	:m_errorhnd(errorhnd_),m_dbgtrace(0),m_patternar(),m_patternrefar(),m_expressionar(),m_variablear(),m_operandsar(),m_stk(),m_done(false)
{
	DebugTraceInterface* dt = m_errorhnd->debugTrace();
	m_dbgtrace = dt ? dt->createTraceContext( STRUS_DBGTRACE_COMPONENT_NAME) : NULL;
}

TestPatternMatcherInstance::~TestPatternMatcherInstance()
{
	if (m_dbgtrace) delete m_dbgtrace;
}

void TestPatternMatcherInstance::defineOption( const std::string& name, double value)
{
	try
	{
		if (m_dbgtrace) m_dbgtrace->event( "option", "%s %f", name.c_str(), value);
		if (m_done) throw strus::runtime_error(_TXT("illegal call of %s"), "defineOption");
		throw std::runtime_error("unknonw option passed to pattern lexer");
	}
	CATCH_ERROR_ARG1_MAP( _TXT("error calling %s: %s"), "TestPatternMatcherInstance::defineOption", *m_errorhnd);
}

void TestPatternMatcherInstance::defineTermFrequency( unsigned int termid, double df)
{
	try
	{
		if (m_dbgtrace) m_dbgtrace->event( "df", "%d %f", termid, df);
		if (m_done) throw strus::runtime_error(_TXT("illegal call of %s"), "defineTermFrequency");
	}
	CATCH_ERROR_ARG1_MAP( _TXT("error calling %s: %s"), "TestPatternMatcherInstance::defineTermFrequency", *m_errorhnd);
}

void TestPatternMatcherInstance::pushTerm( unsigned int termid)
{
	try
	{
		if (m_dbgtrace) m_dbgtrace->event( "push", "term %d", termid);
		if (m_done) throw strus::runtime_error(_TXT("illegal call of %s"), "pushTerm");
		if (termid >= MaxId) throw std::runtime_error("illegal term id pushed");
		m_stk.push_back( termid);
	}
	CATCH_ERROR_ARG1_MAP( _TXT("error calling %s: %s"), "TestPatternMatcherInstance::pushTerm", *m_errorhnd);
}

void TestPatternMatcherInstance::pushExpression(
		JoinOperation operation,
		std::size_t argc, unsigned int range, unsigned int cardinality)
{
	try
	{
		if (m_dbgtrace) m_dbgtrace->event( "push", "expression %s %d %u %u", joinOperationName(operation), (int)argc, range, cardinality);
		if (m_done) throw strus::runtime_error(_TXT("illegal call of %s"), "pushExpression");
		if (m_stk.size() < argc) throw std::runtime_error("illegal operation");
		if (m_expressionar.size() > MaxId) throw std::runtime_error("too many expressions pushed");
	
		int operandsidx = m_operandsar.size();
		m_operandsar.insert( m_operandsar.end(), m_stk.end() - argc, m_stk.end());
		m_stk.resize( m_stk.size() - argc);
		m_stk.push_back( m_expressionar.size() + ExpressionIdOfs);
		m_expressionar.push_back( Expression( operation, argc, range, cardinality, operandsidx));
	}
	CATCH_ERROR_ARG1_MAP( _TXT("error calling %s: %s"), "TestPatternMatcherInstance::pushExpression", *m_errorhnd);
}

void TestPatternMatcherInstance::pushPattern( const std::string& name)
{
	try
	{
		if (m_dbgtrace) m_dbgtrace->event( "push", "pattern %s", name.c_str());
		if (m_done) throw strus::runtime_error(_TXT("illegal call of %s"), "pushPattern");
		if (m_stk.empty()) throw std::runtime_error("illegal operation");
		if (m_patternar.size() > MaxId) throw std::runtime_error("too many expressions pushed");
	
		m_stk.push_back( m_patternrefar.size() + PatternIdOfs);
		m_patternrefar.push_back( name);
	}
	CATCH_ERROR_ARG1_MAP( _TXT("error calling %s: %s"), "TestPatternMatcherInstance::pushPattern", *m_errorhnd);
}

void TestPatternMatcherInstance::attachVariable( const std::string& name)
{
	try
	{
		if (m_dbgtrace) m_dbgtrace->event( "attach", "variable %s", name.c_str());
		if (m_done) throw strus::runtime_error(_TXT("illegal call of %s"), "attachVariable");
		if (m_stk.empty()) throw std::runtime_error("illegal operation");
		m_variablear.push_back( Variable( m_stk.back(), name));
	}
	CATCH_ERROR_ARG1_MAP( _TXT("error calling %s: %s"), "TestPatternMatcherInstance::attachVariable", *m_errorhnd);
}

void TestPatternMatcherInstance::definePattern( const std::string& name, bool visible)
{
	try
	{
		if (m_dbgtrace) m_dbgtrace->event( "pattern", "%s %s", name.c_str(), visible?"public":"private");
		if (m_done) throw strus::runtime_error(_TXT("illegal call of %s"), "definePattern");
		m_patternar.push_back( Pattern( m_stk.back(), name));
		m_stk.pop_back();
	}
	CATCH_ERROR_ARG1_MAP( _TXT("error calling %s: %s"), "TestPatternMatcherInstance::definePattern", *m_errorhnd);
}

bool TestPatternMatcherInstance::compile()
{
	try
	{
		if (m_dbgtrace) m_dbgtrace->event( "compile", "patterns %d expressions %d variables %d patternrefs %d",
							(int)m_patternar.size(), (int)m_expressionar.size(), (int)m_variablear.size(), (int)m_patternrefar.size());
		if (m_done) throw strus::runtime_error(_TXT("already called %s"), "compile");
		m_done = true;
		return true;
	}
	CATCH_ERROR_ARG1_MAP_RETURN( _TXT("error calling %s: %s"), "TestPatternMatcherInstance::compile", *m_errorhnd, false);
}

PatternMatcherContextInterface* TestPatternMatcherInstance::createContext() const
{
	try
	{
		return new TestPatternMatcherContext( m_errorhnd, this);
	}
	CATCH_ERROR_ARG1_MAP_RETURN( _TXT("error calling %s: %s"), "TestPatternMatcherInstance::createContext", *m_errorhnd, NULL);
}

enum ItemType {
	ItemToken,
	ItemPatternRef,
	ItemExpression
};
static ItemType getItemType( unsigned int id, unsigned int& idx)
{
	if (id >= ExpressionIdOfs) {idx = id-ExpressionIdOfs; return ItemExpression;}
	if (id >= PatternIdOfs) {idx = id-PatternIdOfs; return ItemPatternRef;}
	idx = id;
	return ItemToken;
}

const char* TestPatternMatcherInstance::getVariableAttached( unsigned int id) const
{
	std::vector<Variable>::const_iterator vi = m_variablear.begin(), ve = m_variablear.end();
	for (; vi != ve; ++vi)
	{
		if (vi->id == id) return vi->name.c_str();
	}
	return NULL;
}

std::vector<unsigned int> TestPatternMatcherInstance::getPatternRefs( const std::string& patternName) const
{
	std::vector<unsigned int> rt;
	std::vector<Pattern>::const_iterator pi = m_patternar.begin(), pe = m_patternar.end();
	for (; pi != pe; ++pi)
	{
		if (pi->name == patternName) rt.push_back( pi->id);
	}
	return rt;
}


TestPatternMatcherContext::TestPatternMatcherContext( ErrorBufferInterface* errorhnd_, const TestPatternMatcherInstance* instance_)
	:m_errorhnd(errorhnd_),m_dbgtrace(0),m_dbgtrace_proc(0),m_instance(instance_)
{
	DebugTraceInterface* dt = m_errorhnd->debugTrace();
	m_dbgtrace = dt ? dt->createTraceContext( STRUS_DBGTRACE_COMPONENT_NAME) : NULL;
	m_dbgtrace_proc = dt ? dt->createTraceContext( STRUS_DBGTRACE_COMPONENT_NAME_PROC) : NULL;
}

TestPatternMatcherContext::~TestPatternMatcherContext()
{
	if (m_dbgtrace) delete m_dbgtrace;
	if (m_dbgtrace_proc) delete m_dbgtrace_proc;
}

void TestPatternMatcherContext::putInput( const analyzer::PatternLexem& token)
{
	try
	{
		m_inputar.push_back( token);
	}
	CATCH_ERROR_ARG1_MAP( _TXT("error calling %s: %s"), "TestPatternMatcherContext::putInput", *m_errorhnd);
}

static bool comparePatternMatcherResult( const analyzer::PatternMatcherResult& aa, const analyzer::PatternMatcherResult& bb)
{
	if (aa.start_ordpos() < bb.start_ordpos()) return true;
	if (aa.start_ordpos() > bb.start_ordpos()) return false;
	if (aa.start_origpos() < bb.start_origpos()) return true;
	if (aa.start_origpos() > bb.start_origpos()) return false;
	if (aa.end_ordpos() > bb.end_ordpos()) return true;
	if (aa.end_ordpos() < bb.end_ordpos()) return false;
	return std::strcmp( aa.name(), bb.name()) < 0;
}

std::vector<analyzer::PatternMatcherResult> TestPatternMatcherContext::fetchResults() const
{
	try
	{
		std::vector<analyzer::PatternMatcherResult> rt;
		std::vector<TestPatternMatcherInstance::Pattern>::const_iterator pi = m_instance->m_patternar.begin(), pe = m_instance->m_patternar.end();
		for (; pi != pe; ++pi)
		{
			evalPattern( rt, *pi);
		}
		std::sort( rt.begin(), rt.end(), comparePatternMatcherResult);
		return rt;
	}
	CATCH_ERROR_ARG1_MAP_RETURN( _TXT("error calling %s: %s"), "TestPatternMatcherContext::fetchResults", *m_errorhnd, std::vector<analyzer::PatternMatcherResult>());
}

analyzer::PatternMatcherStatistics TestPatternMatcherContext::getStatistics() const
{
	return analyzer::PatternMatcherStatistics();
}

void TestPatternMatcherContext::reset()
{
	m_inputar.clear();
}


void TestPatternMatcherContext::joinResult( MatchResult& result, const MatchResult& aresult)
{
	if (aresult.match.ordpos + aresult.match.ordlen >  result.match.ordpos + result.match.ordlen)
	{
		result.match.ordlen = aresult.match.ordpos + aresult.match.ordlen - result.match.ordpos;
	}
	if (aresult.match.start < result.match.start)
	{
		result.match.start = aresult.match.start;
	}
	if (aresult.match.end > result.match.end)
	{
		result.match.end = aresult.match.end;
	}
	result.items.insert( result.items.end(), aresult.items.begin(), aresult.items.end());
}

bool TestPatternMatcherContext::findFirstMatch( MatchResult& result, unsigned int id, int inputiter, unsigned int maxordlen, bool imm, bool seq) const
{
	for (std::size_t ii=inputiter; ii < m_inputar.size(); ++ii)
	{
		if (maxordlen)
		{
			const analyzer::PatternLexem& token = m_inputar[ ii];
			if (result.match.ordpos + result.match.ordlen - token.ordpos() > maxordlen) return false;
		}
		MatchResult aresult;
		if (!matchItem( aresult, id, inputiter)) continue;
		if (seq)
		{
			if (imm)
			{
				if (aresult.match.ordpos != result.match.ordpos + result.match.ordlen) continue;
			}
			else
			{
				if (aresult.match.ordpos < result.match.ordpos + result.match.ordlen) continue;
			}
			if (maxordlen)
			{
				int endordpos = std::max( aresult.match.ordpos + aresult.match.ordlen, result.match.ordpos + result.match.ordlen);
				if (endordpos < result.match.ordpos + (int)maxordlen) continue;
			}
			joinResult( result, aresult);
			return true;
		}
		else
		{
			if (imm)
			{
				throw std::runtime_error("immediate not implemented for within");
			}
			if (maxordlen)
			{
				int endordpos = std::max( aresult.match.ordpos + aresult.match.ordlen, result.match.ordpos + result.match.ordlen);
				if (endordpos < result.match.ordpos + (int)maxordlen) continue;
			}
			joinResult( result, aresult);
			return true;
		}
	}
	return false;
}

bool TestPatternMatcherContext::matchCombined( MatchResult& result, unsigned int structid, const unsigned int* idar, int size, int inputiter, unsigned int maxordlen, bool imm, bool seq, unsigned int cardinality) const
{
	int nofMatches = 0;
	if (seq && cardinality) throw std::runtime_error( "cardinality not implemented for sequence");

	unsigned int const* pi = idar;
	const unsigned int* pe = pi + size;
	if (structid)
	{
		if (!size) return false;
		++pi;
	}
	int pidx = 0;
	for (; pi < pe; ++pi,++pidx)
	{
		if (!pidx)
		{
			if (!matchItem( result, *pi, inputiter)) return false;
		}
		else
		{
			if (findFirstMatch( result, *pi, inputiter, maxordlen, imm, seq))
			{
				++nofMatches;
			}
		}
	}
	if (maxordlen && result.match.ordlen > (int)maxordlen) return false;
	if (structid)
	{
		MatchResult aresult = result;
		if (findFirstMatch( aresult, *pi, inputiter, result.match.ordlen, false, false)) return false;
	}
	if (cardinality ? nofMatches >= (int)cardinality : nofMatches == pidx)
	{
		if (m_dbgtrace_proc) m_dbgtrace_proc->event( "matchcombined", "%s%s%s pos %d nof %d cardinality %d",
					(seq?"sequence":"within"), (structid?"_struct":""), (imm?"_imm":""),
					result.match.ordpos, nofMatches, cardinality ? cardinality : pidx);
		return true;
	}
	else
	{
		return false;
	}
}

bool TestPatternMatcherContext::matchShortest( MatchResult& result, std::vector<unsigned int>::const_iterator begin, const std::vector<unsigned int>::const_iterator& end, int inputiter) const
{
	int maxordlen = std::numeric_limits<int>::max();
	std::vector<unsigned int>::const_iterator pi = begin;
	for (; pi != end; ++pi)
	{
		MatchResult aresult;
		if (matchItem( aresult, *pi, inputiter) && aresult.match.ordlen < maxordlen)
		{
			result = aresult;
			maxordlen = aresult.match.ordlen;
		}
	}
	if (maxordlen < std::numeric_limits<int>::max())
	{
		if (m_dbgtrace_proc) m_dbgtrace_proc->event( "matchany", "pos %d len %d", result.match.ordpos, result.match.ordlen);
		return true;
	}
	else
	{
		return false;
	}
}

bool TestPatternMatcherContext::matchShortest( MatchResult& result, unsigned int const* begin, const unsigned int* end, int inputiter) const
{
	std::vector<unsigned int> ar( begin, end);
	return matchShortest( result, ar.begin(), ar.end(), inputiter);
}

bool TestPatternMatcherContext::matchAll( MatchResult& result, std::vector<unsigned int>::const_iterator begin, const std::vector<unsigned int>::const_iterator& end, int inputiter, int cardinality) const
{
	int pidx = 0;
	int nofMatches = 0;
	std::vector<unsigned int>::const_iterator pi = begin;
	for (; pi != end; ++pi,++pidx)
	{
		MatchResult aresult;
		if (!matchItem( aresult, *pi, inputiter)) continue;
		if (pidx == 0)
		{
			result = aresult;
		}
		else
		{
			if (aresult.match.ordpos != result.match.ordpos) continue;
			joinResult( result, aresult);
			++nofMatches;
		}
	}
	if (cardinality ? nofMatches >= cardinality : nofMatches == pidx)
	{
		if (m_dbgtrace_proc) m_dbgtrace_proc->event( "matchall", "nof %d cardinality %d", nofMatches, cardinality ? cardinality : pidx);
		return true;
	}
	else
	{
		return false;
	}
}

bool TestPatternMatcherContext::matchAll( MatchResult& result, unsigned int const* begin, const unsigned int* end, int inputiter, int cardinality) const
{
	std::vector<unsigned int> ar( begin, end);
	return matchAll( result, ar.begin(), ar.end(), inputiter, cardinality);
}

bool TestPatternMatcherContext::matchItem( MatchResult& result, unsigned int id, int inputiter) const
{
	unsigned int itemidx = 0;
	switch (getItemType( id, itemidx))
	{
		case ItemToken:
		{
			const analyzer::PatternLexem& token = m_inputar[ inputiter];
			if (token.id() == itemidx)
			{
				MatchAddress startadr( token.origseg(), token.origpos());
				MatchAddress endadr( token.origseg(), token.origpos() + token.origsize());
				Match match( token.ordpos(), 1/*ordlen*/, startadr, endadr);
				result = MatchResult( match);
				const char* variable = m_instance->getVariableAttached( id);
				if (variable)
				{
					if (m_dbgtrace_proc) m_dbgtrace_proc->event( "matchtoken", "id %d pos %d var %s", (int)token.id(), (int)token.ordpos(), variable);
					result.items.push_back( MatchItem( match, variable));
				}
				else
				{
					if (m_dbgtrace_proc) m_dbgtrace_proc->event( "matchtoken", "id %d pos %d", (int)token.id(), (int)token.ordpos());
				}
				return true;
			}
			return false;
		}
		case ItemPatternRef:
		{
			std::vector<unsigned int> patterns = m_instance->getPatternRefs( m_instance->m_patternrefar[ itemidx]);
			return matchShortest( result, patterns.begin(), patterns.end(), inputiter);
		}
		case ItemExpression:
		{
			const TestPatternMatcherInstance::Expression& expression = m_instance->m_expressionar[ itemidx];
			const unsigned int* idar = m_instance->m_operandsar.data() + expression.operandsidx;
			if (expression.argc == 0) return false;

			switch (expression.operation)
			{
				case PatternMatcherInstanceInterface::OpSequence:
					return matchCombined( result, 0/*structid*/, idar, expression.argc, inputiter, expression.range, false/*imm*/, true/*seq*/, expression.cardinality);
				case PatternMatcherInstanceInterface::OpSequenceImm:
					return matchCombined( result, 0/*structid*/, idar, expression.argc, inputiter, expression.range, true/*imm*/, true/*seq*/, expression.cardinality);
				case PatternMatcherInstanceInterface::OpSequenceStruct:
					return matchCombined( result, idar[0]/*structid*/, idar+1, expression.argc-1, inputiter, expression.range, false/*imm*/, true/*seq*/, expression.cardinality);
				case PatternMatcherInstanceInterface::OpWithin:
					return matchCombined( result, 0/*structid*/, idar, expression.argc, inputiter, expression.range, false/*imm*/, false/*seq*/, expression.cardinality);
				case PatternMatcherInstanceInterface::OpWithinStruct:
					return matchCombined( result, idar[0]/*structid*/, idar+1, expression.argc-1, inputiter, expression.range, false/*imm*/, false/*seq*/, expression.cardinality);
				case PatternMatcherInstanceInterface::OpAny:
					return matchShortest( result, idar, idar + expression.argc, inputiter);
				case PatternMatcherInstanceInterface::OpAnd:
					return matchAll( result, idar, idar + expression.argc, expression.cardinality, inputiter);
			}
			return false;
		}
	}
	throw strus::runtime_error(_TXT("internal: unknown item type"));
}

void TestPatternMatcherContext::evalPattern( std::vector<analyzer::PatternMatcherResult>& res, const TestPatternMatcherInstance::Pattern& pattern) const
{
	for (int inputiter=0; inputiter < (int)m_inputar.size(); ++inputiter)
	{
		MatchResult result;
		if (matchItem( result, pattern.id, inputiter))
		{
			std::vector<analyzer::PatternMatcherResultItem> itemar;
			std::vector<MatchItem>::const_iterator mi = result.items.begin(), me = result.items.end();
			for (; mi != me; ++mi)
			{
				itemar.push_back( analyzer::PatternMatcherResultItem(
					mi->variable, mi->ordpos, mi->ordpos+mi->ordlen,
					mi->start.seg, mi->start.pos, mi->end.seg, mi->end.pos));
			}
			analyzer::PatternMatcherResult elem( 
				pattern.name.c_str(), result.match.ordpos, result.match.ordpos + result.match.ordlen,
				result.match.start.seg, result.match.start.pos,
				result.match.end.seg, result.match.end.pos, itemar);
			res.push_back( elem);
		}
	}
}





