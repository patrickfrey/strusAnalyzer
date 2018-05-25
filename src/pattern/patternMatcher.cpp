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
#include "strus/lib/pattern_resultformat.hpp"
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
	MaxId=(1<<28),
	PatternIdOfs=(1<<28),
	ExpressionIdOfs=(1<<29)
};

PatternMatcherInstanceInterface* TestPatternMatcher::createInstance() const
{
	try
	{
		return new TestPatternMatcherInstance( m_errorhnd);
	}
	CATCH_ERROR_ARG1_MAP_RETURN( _TXT("error creating %s: %s"), "TestPatternMatcherInstance", *m_errorhnd, NULL);
}


TestPatternMatcherInstance::TestPatternMatcherInstance( ErrorBufferInterface* errorhnd_)
	:m_errorhnd(errorhnd_),m_debugtrace(0),m_patternar(),m_patternrefar()
	,m_expressionar(),m_exprvarmap(),m_varmap(),m_resultFormatTable(0)
	,m_operandsar(),m_stk(),m_exprfmtmap(),m_done(false)
{
	DebugTraceInterface* dt = m_errorhnd->debugTrace();
	m_debugtrace = dt ? dt->createTraceContext( STRUS_DBGTRACE_COMPONENT_NAME) : NULL;
	m_resultFormatTable = new PatternResultFormatTable( m_errorhnd, &m_varmap);
}

TestPatternMatcherInstance::~TestPatternMatcherInstance()
{
	if (m_debugtrace) delete m_debugtrace;
	if (m_resultFormatTable) delete m_resultFormatTable;
}

const char* TestPatternMatcherInstance::VariableMap::getVariable( const std::string& name) const
{
	int symid = m_map.get( name);
	if (!symid) return NULL;
	return m_map.key( symid);
}

const char* TestPatternMatcherInstance::VariableMap::getOrCreateVariable( const std::string& name)
{
	int symid = m_map.getOrCreate( name);
	if (!symid) return NULL;
	return m_map.key( symid);
}

const char* TestPatternMatcherInstance::getVariableId( const std::string& name) const
{
	return m_varmap.getVariable( name);
}

void TestPatternMatcherInstance::defineOption( const std::string& name, double value)
{
	try
	{
		if (m_debugtrace) m_debugtrace->event( "option", "%s %f", name.c_str(), value);
		if (m_done) throw strus::runtime_error(_TXT("illegal call of %s"), "defineOption");
		throw std::runtime_error("unknonw option passed to pattern lexer");
	}
	CATCH_ERROR_ARG1_MAP( _TXT("error calling %s: %s"), "TestPatternMatcherInstance::defineOption", *m_errorhnd);
}

void TestPatternMatcherInstance::defineTermFrequency( unsigned int termid, double df)
{
	try
	{
		if (m_debugtrace) m_debugtrace->event( "df", "%d %f", termid, df);
		if (m_done) throw strus::runtime_error(_TXT("illegal call of %s"), "defineTermFrequency");
	}
	CATCH_ERROR_ARG1_MAP( _TXT("error calling %s: %s"), "TestPatternMatcherInstance::defineTermFrequency", *m_errorhnd);
}

void TestPatternMatcherInstance::pushTerm( unsigned int termid)
{
	try
	{
		if (m_debugtrace) m_debugtrace->event( "push", "term %d", termid);
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
		if (m_debugtrace) m_debugtrace->event( "push", "expression %s %d %u %u", joinOperationName(operation), (int)argc, range, cardinality);
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
		if (m_debugtrace) m_debugtrace->event( "push", "pattern %s", name.c_str());
		if (m_done) throw strus::runtime_error(_TXT("illegal call of %s"), "pushPattern");
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
		if (m_debugtrace) m_debugtrace->event( "attach", "variable %s", name.c_str());
		if (m_done) throw strus::runtime_error(_TXT("illegal call of %s"), "attachVariable");
		if (m_stk.empty()) throw std::runtime_error("illegal operation");
		const char* variable = m_varmap.getOrCreateVariable( name);
		if (!variable) throw std::bad_alloc();
		m_exprvarmap[ m_stk.back()] = variable;
	}
	CATCH_ERROR_ARG1_MAP( _TXT("error calling %s: %s"), "TestPatternMatcherInstance::attachVariable", *m_errorhnd);
}

void TestPatternMatcherInstance::definePattern( const std::string& name, const std::string& formatstring, bool visible)
{
	try
	{
		if (m_debugtrace) m_debugtrace->event( "pattern", "%s %s", name.c_str(), visible?"public":"private");
		if (m_done) throw strus::runtime_error(_TXT("illegal call of %s"), "definePattern");
		if (m_stk.empty()) throw std::runtime_error("illegal operation");
		m_patternar.push_back( Pattern( m_stk.back(), name));
		if (!formatstring.empty())
		{
			const PatternResultFormat* fmt = m_resultFormatTable->createResultFormat( formatstring.c_str());
			if (!fmt) throw std::runtime_error( m_errorhnd->fetchError());
			m_exprfmtmap[ m_stk.back()] = fmt;
		}
		m_stk.pop_back();
	}
	CATCH_ERROR_ARG1_MAP( _TXT("error calling %s: %s"), "TestPatternMatcherInstance::definePattern", *m_errorhnd);
}

bool TestPatternMatcherInstance::compile()
{
	try
	{
		if (m_debugtrace) m_debugtrace->event( "compile", "patterns %d expressions %d variables %d patternrefs %d",
							(int)m_patternar.size(), (int)m_expressionar.size(), (int)m_varmap.size(), (int)m_patternrefar.size());
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
	ExpressionVariableMap::const_iterator vi = m_exprvarmap.find( id);
	if (vi != m_exprvarmap.end())
	{
		return vi->second;
	}
	return NULL;
}

const PatternResultFormat* TestPatternMatcherInstance::getResultFormat( unsigned int id) const
{
	ExpressionResultFormatMap::const_iterator vi = m_exprfmtmap.find( id);
	if (vi != m_exprfmtmap.end())
	{
		return vi->second;
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

analyzer::FunctionView TestPatternMatcherInstance::view() const
{
	try
	{
		return analyzer::FunctionView( "testmatcher")
		;
	}
	CATCH_ERROR_MAP_RETURN( _TXT("error in introspection: %s"), *m_errorhnd, analyzer::FunctionView());
}

TestPatternMatcherContext::TestPatternMatcherContext( ErrorBufferInterface* errorhnd_, const TestPatternMatcherInstance* instance_)
	:m_errorhnd(errorhnd_),m_debugtrace(0),m_debugtrace_proc(0),m_instance(instance_),m_resultFormatContext(errorhnd_),m_inputar(),m_values()
{
	DebugTraceInterface* dt = m_errorhnd->debugTrace();
	m_debugtrace = dt ? dt->createTraceContext( STRUS_DBGTRACE_COMPONENT_NAME) : NULL;
	m_debugtrace_proc = dt ? dt->createTraceContext( STRUS_DBGTRACE_COMPONENT_NAME_PROC) : NULL;
}

TestPatternMatcherContext::~TestPatternMatcherContext()
{
	if (m_debugtrace) delete m_debugtrace;
	if (m_debugtrace_proc) delete m_debugtrace_proc;
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

std::vector<analyzer::PatternMatcherResult> TestPatternMatcherContext::fetchResults()
{
	try
	{
		std::vector<analyzer::PatternMatcherResult> rt;
		std::vector<TestPatternMatcherInstance::Pattern>::const_iterator pi = m_instance->m_patternar.begin(), pe = m_instance->m_patternar.end();
		for (; pi != pe; ++pi)
		{
			if (m_debugtrace_proc) m_debugtrace_proc->event( "evalpattern", "name %s id %d", pi->name.c_str(), (int)pi->id);
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

const char* TestPatternMatcherContext::allocCharp( const std::string& value)
{
	m_values.push_back( value);
	return m_values.back().c_str();
}

std::string TestPatternMatcherContext::mapResultValue( unsigned int id, const MatchResult& result)
{
	const PatternResultFormat* fmt = m_instance->getResultFormat( id);
	if (fmt)
	{
		std::vector<analyzer::PatternMatcherResultItem> ar;
		std::vector<strus::Reference<MatchResult> >::const_iterator ii = result.items.begin(), ie = result.items.end();
		for (; ii != ie; ++ii)
		{
			const char* variable = m_instance->getVariableId( (*ii)->name);
			if (variable)
			{
				analyzer::PatternMatcherResultItem translated( variable, (*ii)->value.c_str(), (*ii)->ordpos, (*ii)->ordpos+(*ii)->ordlen, (*ii)->start.seg, (*ii)->start.pos, (*ii)->end.seg, (*ii)->end.pos);
				ar.push_back( translated);
			}
		}
		const char* valptr = m_resultFormatContext.map( fmt, ar.data(), ar.size());
		if (!valptr) throw std::runtime_error( m_errorhnd->fetchError());
		return std::string( valptr);
	}
	else
	{
		return std::string();
	}
}

void TestPatternMatcherContext::joinResult( MatchResult& result, unsigned int id, const MatchResult& aresult)
{
	if (!result.defined())
	{
		result.ordpos = aresult.ordpos;
		result.ordlen = aresult.ordlen;
		result.start = aresult.start;
		result.end = aresult.end;
	}
	else
	{
		if (aresult.ordpos + aresult.ordlen >  result.ordpos + result.ordlen)
		{
			result.ordlen = aresult.ordpos + aresult.ordlen - result.ordpos;
		}
		if (aresult.start < result.start)
		{
			result.start = aresult.start;
		}
		if (aresult.end > result.end)
		{
			result.end = aresult.end;
		}
	}
	const char* variable = m_instance->getVariableAttached( id);
	if (variable)
	{
		std::string value = mapResultValue( id, aresult);
		result.items.push_back( new MatchResult( variable, value, aresult.ordpos, aresult.ordlen, aresult.start, aresult.end));
	}
	else
	{
		result.items.insert( result.items.end(), aresult.items.begin(), aresult.items.end());
	}
}

bool TestPatternMatcherContext::findFirstMatch( MatchResult& result, unsigned int id, int inputiter, unsigned int maxordlen, bool imm, bool seq)
{
	for (; inputiter < (int)m_inputar.size(); ++inputiter)
	{
		if (maxordlen)
		{
			const analyzer::PatternLexem& token = m_inputar[ inputiter];
			if (token.ordpos() - result.ordpos > maxordlen) return false;
		}
		MatchResult aresult;
		if (!matchItem( aresult, id, inputiter)) continue;
		if (seq)
		{
			if (imm)
			{
				if (aresult.ordpos != result.ordpos + result.ordlen) continue;
			}
			else
			{
				if (aresult.ordpos < result.ordpos + result.ordlen) continue;
			}
			if (maxordlen)
			{
				int endordpos = std::max( aresult.ordpos + aresult.ordlen, result.ordpos + result.ordlen);
				if (endordpos > result.ordpos + (int)maxordlen) continue;
			}
			joinResult( result, id, aresult);
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
				int endordpos = std::max( aresult.ordpos + aresult.ordlen, result.ordpos + result.ordlen);
				if (endordpos > result.ordpos + (int)maxordlen) continue;
			}
			joinResult( result, id, aresult);
			return true;
		}
	}
	return false;
}

bool TestPatternMatcherContext::matchCombined( MatchResult& result, unsigned int structid, const unsigned int* idar, int idarsize, int inputiter, unsigned int maxordlen, bool imm, bool seq, unsigned int cardinality)
{
	int nofMatches = 0;
	if (seq && cardinality) throw std::runtime_error( "cardinality not implemented for sequence");

	unsigned int const* pi = idar;
	const unsigned int* pe = pi + idarsize;
	int pidx = 0;
	for (; pi < pe; ++pi,++pidx)
	{
		if (!pidx)
		{
			if (!matchItem( result, *pi, inputiter)) return false;
			++nofMatches;
		}
		else
		{
			if (findFirstMatch( result, *pi, inputiter, maxordlen, imm, seq))
			{
				++nofMatches;
			}
		}
	}
	if (maxordlen && result.ordlen > (int)maxordlen) return false;
	if (structid)
	{
		MatchResult aresult = result;
		if (findFirstMatch( aresult, *pi, inputiter, result.ordlen, false, false)) return false;
	}
	if (cardinality ? nofMatches >= (int)cardinality : nofMatches == pidx)
	{
		if (m_debugtrace_proc) m_debugtrace_proc->event( "matchcombined", "%s%s%s pos %d len %d nof %d cardinality %d",
					(seq?"sequence":"within"), (structid?"_struct":""), (imm?"_imm":""),
					result.ordpos, result.ordlen, nofMatches, cardinality ? cardinality : pidx);
		return true;
	}
	else
	{
		return false;
	}
}

bool TestPatternMatcherContext::matchShortest( MatchResult& result, std::vector<unsigned int>::const_iterator begin, const std::vector<unsigned int>::const_iterator& end, int inputiter)
{
	MatchResult bestresult;
	int bestpt = -1;
	int maxordlen = std::numeric_limits<int>::max();
	std::vector<unsigned int>::const_iterator pi = begin;
	for (; pi != end; ++pi)
	{
		MatchResult aresult;
		if (matchItem( aresult, *pi, inputiter) && aresult.ordlen < maxordlen)
		{
			bestresult = aresult;
			bestpt = *pi;
			maxordlen = aresult.ordlen;
		}
	}
	if (maxordlen < std::numeric_limits<int>::max())
	{
		joinResult( result, bestpt, bestresult);
		if (m_debugtrace_proc) m_debugtrace_proc->event( "matchany", "pos %d len %d", result.ordpos, result.ordlen);
		return true;
	}
	else
	{
		return false;
	}
}

bool TestPatternMatcherContext::matchShortest( MatchResult& result, const unsigned int* begin, const unsigned int* end, int inputiter)
{
	std::vector<unsigned int> ar( begin, end);
	return matchShortest( result, ar.begin(), ar.end(), inputiter);
}

bool TestPatternMatcherContext::matchAll( MatchResult& result, std::vector<unsigned int>::const_iterator begin, const std::vector<unsigned int>::const_iterator& end, int inputiter, int cardinality)
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
			joinResult( result, *pi, aresult);
		}
		else
		{
			if (aresult.ordpos != result.ordpos) continue;
			joinResult( result, *pi, aresult);
			++nofMatches;
		}
	}
	if (cardinality ? nofMatches >= cardinality : nofMatches == pidx)
	{
		if (m_debugtrace_proc) m_debugtrace_proc->event( "matchall", "nof %d cardinality %d", nofMatches, cardinality ? cardinality : pidx);
		return true;
	}
	else
	{
		return false;
	}
}

bool TestPatternMatcherContext::matchAll( MatchResult& result, const unsigned int* begin, const unsigned int* end, int inputiter, int cardinality)
{
	std::vector<unsigned int> ar( begin, end);
	return matchAll( result, ar.begin(), ar.end(), inputiter, cardinality);
}

bool TestPatternMatcherContext::matchItem( MatchResult& result, unsigned int id, int inputiter)
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
				result = MatchResult( "", "", token.ordpos(), 1/*ordlen*/, startadr, endadr);
				return true;
			}
			return false;
		}
		case ItemPatternRef:
		{
			bool rt = false;
			std::vector<unsigned int> patterns = m_instance->getPatternRefs( m_instance->m_patternrefar[ itemidx]);
			if (patterns.size() == 1)
			{
				rt = (matchItem( result, patterns[0], inputiter));
			}
			else if (!patterns.empty())
			{
				rt = matchShortest( result, patterns.begin(), patterns.end(), inputiter);
			}
			return rt;
		}
		case ItemExpression:
		{
			bool rt = false;
			const TestPatternMatcherInstance::Expression& expression = m_instance->m_expressionar[ itemidx];
			unsigned int const* idar = m_instance->m_operandsar.data() + expression.operandsidx;
			if (expression.argc == 0) return false;

			switch (expression.operation)
			{
				case PatternMatcherInstanceInterface::OpSequence:
					rt = matchCombined( result, 0/*structid*/, idar, expression.argc, inputiter, expression.range, false/*imm*/, true/*seq*/, expression.cardinality);
					break;
				case PatternMatcherInstanceInterface::OpSequenceImm:
					rt = matchCombined( result, 0/*structid*/, idar, expression.argc, inputiter, expression.range, true/*imm*/, true/*seq*/, expression.cardinality);
					break;
				case PatternMatcherInstanceInterface::OpSequenceStruct:
					rt = matchCombined( result, idar[0]/*structid*/, idar+1, expression.argc-1, inputiter, expression.range, false/*imm*/, true/*seq*/, expression.cardinality);
					break;
				case PatternMatcherInstanceInterface::OpWithin:
					rt = matchCombined( result, 0/*structid*/, idar, expression.argc, inputiter, expression.range, false/*imm*/, false/*seq*/, expression.cardinality);
					break;
				case PatternMatcherInstanceInterface::OpWithinStruct:
					rt = matchCombined( result, idar[0]/*structid*/, idar+1, expression.argc-1, inputiter, expression.range, false/*imm*/, false/*seq*/, expression.cardinality);
					break;
				case PatternMatcherInstanceInterface::OpAny:
					rt = matchShortest( result, idar, idar + expression.argc, inputiter);
					break;
				case PatternMatcherInstanceInterface::OpAnd:
					rt = matchAll( result, idar, idar + expression.argc, expression.cardinality, inputiter);
					break;
			}
			return rt;
		}
	}
	throw strus::runtime_error(_TXT("internal: unknown item type"));
}

void TestPatternMatcherContext::evalPattern( std::vector<analyzer::PatternMatcherResult>& res,
						const TestPatternMatcherInstance::Pattern& pattern)
{
	for (int inputiter=0; inputiter < (int)m_inputar.size(); ++inputiter)
	{
		MatchResult result;
		if (m_debugtrace_proc)
		{
			const analyzer::PatternLexem& lexem = m_inputar[ inputiter];
			m_debugtrace_proc->event( "token", "[%d] id %d pos %d", inputiter, (int)lexem.id(), (int)lexem.ordpos());
		}
		if (matchItem( result, pattern.id, inputiter))
		{
			if (m_debugtrace) m_debugtrace->event( "pattern", "id %d name %s at %d length %d", (int)pattern.id, pattern.name.c_str(), result.ordpos, result.ordlen);

			std::vector<analyzer::PatternMatcherResultItem> itemar;
			std::vector<strus::Reference<MatchResult> >::const_iterator mi = result.items.begin(), me = result.items.end();
			for (; mi != me; ++mi)
			{
				const char* variable = m_instance->getVariableId( mi->get()->name);
				const char* name = variable ? variable : allocCharp( mi->get()->name);
				itemar.push_back( analyzer::PatternMatcherResultItem(
					name, allocCharp( (*mi)->value),
					(*mi)->ordpos, (*mi)->ordpos+(*mi)->ordlen,
					(*mi)->start.seg, (*mi)->start.pos, (*mi)->end.seg, (*mi)->end.pos));
			}
			const char* itemValue = 0;
			const PatternResultFormat* fmt = m_instance->getResultFormat( pattern.id);
			if (fmt)
			{
				itemValue = m_resultFormatContext.map( fmt, itemar.data(), itemar.size());
				itemar.clear();
			}
			analyzer::PatternMatcherResult elem( 
				pattern.name.c_str(), itemValue,
				result.ordpos, result.ordpos + result.ordlen,
				result.start.seg, result.start.pos,
				result.end.seg, result.end.pos, itemar);
			res.push_back( elem);
		}
	}
}





