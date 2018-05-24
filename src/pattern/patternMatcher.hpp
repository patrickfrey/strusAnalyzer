/*
 * Copyright (c) 2018 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Test implementation of the lexer interface for pattern matching
/// \file "patternMatcher.hpp"
#ifndef _STRUS_ANALYZER_TEST_MATCHER_IMPL_HPP_INCLUDED
#define _STRUS_ANALYZER_TEST_MATCHER_IMPL_HPP_INCLUDED
#include "strus/patternMatcherInterface.hpp"
#include "strus/patternMatcherInstanceInterface.hpp"
#include "strus/patternMatcherContextInterface.hpp"
#include "strus/analyzer/patternLexem.hpp"
#include "strus/analyzer/functionView.hpp"
#include "strus/lib/pattern_resultformat.hpp"
#include "strus/base/symbolTable.hpp"
#include <stdexcept>
#include <map>
#include <set>
#include <algorithm>

namespace strus {

///\brief Forward declaration
class ErrorBufferInterface;
///\brief Forward declaration
class DebugTraceContextInterface;

/// \brief Test implementation of the matcher interface for pattern matching
class TestPatternMatcher
	:public PatternMatcherInterface
{
public:
	TestPatternMatcher( ErrorBufferInterface* errorhnd_)
		:m_errorhnd(errorhnd_){}
	virtual ~TestPatternMatcher(){}

	virtual std::vector<std::string> getCompileOptionNames() const		{return std::vector<std::string>();}
	virtual PatternMatcherInstanceInterface* createInstance() const;
	virtual const char* getDescription() const				{return "test pattern matcher";}

private:
	ErrorBufferInterface* m_errorhnd;
};

/// \brief Test implementation of the matcher instance interface for pattern matching
class TestPatternMatcherInstance
	:public PatternMatcherInstanceInterface
{
public:
	TestPatternMatcherInstance( ErrorBufferInterface* errorhnd_);

	virtual ~TestPatternMatcherInstance();
	virtual void defineOption( const std::string& name, double value);
	virtual void defineTermFrequency( unsigned int termid, double df);
	virtual void pushTerm( unsigned int termid);
	virtual void pushExpression(
			JoinOperation operation,
			std::size_t argc, unsigned int range, unsigned int cardinality);
	virtual void pushPattern( const std::string& name);
	virtual void attachVariable( const std::string& name);
	virtual void definePattern( const std::string& name, const std::string& formatstring, bool visible);
	virtual bool compile();

	virtual PatternMatcherContextInterface* createContext() const;
	virtual analyzer::FunctionView view() const;

private:
	friend class TestPatternMatcherContext;

	struct Expression
	{
		JoinOperation operation;
		unsigned int argc;
		unsigned int range;
		unsigned int cardinality;
		int operandsidx;

		Expression( const Expression& o)
			:operation(o.operation),argc(o.argc),range(o.range),cardinality(o.cardinality),operandsidx(o.operandsidx){}
		Expression( JoinOperation operation_, unsigned int argc_, unsigned int range_, unsigned int cardinality_, int operandsidx_)
			:operation(operation_),argc(argc_),range(range_),cardinality(cardinality_),operandsidx(operandsidx_){}
	};
	struct Pattern
	{
		unsigned int id;
		std::string name;
		const PatternResultFormat* fmt;

		Pattern( unsigned int id_, const std::string& name_, const PatternResultFormat* fmt_)
			:id(id_),name(name_),fmt(fmt_){}
		Pattern( const Pattern& o)
			:id(o.id),name(o.name),fmt(o.fmt){}
	};
	typedef std::map<unsigned int,const char*> ExpressionVariableMap;

	class VariableMap
		:public PatternResultFormatVariableMap
	{
	public:
		VariableMap(){}
		virtual ~VariableMap(){}
	
		virtual const char* getVariable( const std::string& name) const;
		const char* getOrCreateVariable( const std::string& name);
		std::size_t size() const	{return m_map.size();}

	private:
		SymbolTable m_map;
	};

	const char* getVariableAttached( unsigned int id) const;
	std::vector<unsigned int> getPatternRefs( const std::string& patternName) const;

private:
	ErrorBufferInterface* m_errorhnd;
	DebugTraceContextInterface* m_debugtrace;
	std::vector<Pattern> m_patternar;
	std::vector<std::string> m_patternrefar;
	std::vector<Expression> m_expressionar;
	ExpressionVariableMap m_exprvarmap;
	VariableMap m_varmap;
	PatternResultFormatTable* m_resultFormatTable;
	std::vector<unsigned int> m_operandsar;
	std::vector<unsigned int> m_stk;
	bool m_done;
};

/// \brief Test implementation of the matcher context interface for pattern matching
class TestPatternMatcherContext
	:public PatternMatcherContextInterface
{
public:
	TestPatternMatcherContext( ErrorBufferInterface* errorhnd_, const TestPatternMatcherInstance* instance_);

	virtual ~TestPatternMatcherContext();
	virtual void putInput( const analyzer::PatternLexem& token);
	virtual std::vector<analyzer::PatternMatcherResult> fetchResults();
	virtual analyzer::PatternMatcherStatistics getStatistics() const;
	virtual void reset();

private:
	struct MatchAddress
	{
		int seg;
		int pos;

		MatchAddress()
			:seg(-1),pos(-1){}
		MatchAddress( int seg_, int pos_)
			:seg(seg_),pos(pos_){}
		MatchAddress( const MatchAddress& o)
			:seg(o.seg),pos(o.pos){}

		bool operator < (const MatchAddress& o) const
		{
			return (seg == o.seg) ? pos < o.pos : seg < o.seg;
		}
		bool operator > (const MatchAddress& o) const
		{
			return (seg == o.seg) ? pos > o.pos : seg > o.seg;
		}
	};

	struct Match
	{
		int ordpos;
		int ordlen;
		MatchAddress start;
		MatchAddress end;

		Match()
			:ordpos(-1),ordlen(-1),start(),end(){}
		Match( int ordpos_, int ordlen_, const MatchAddress& start_, const MatchAddress& end_)
			:ordpos(ordpos_),ordlen(ordlen_),start(start_),end(end_){}
		Match( const Match& o)
			:ordpos(o.ordpos),ordlen(o.ordlen),start(o.start),end(o.end){}
		Match& operator = (const Match& o) {ordpos=o.ordpos;ordlen=o.ordlen;start=o.start;end=o.end; return *this;}
	};

	struct MatchItem
		:public Match
	{
		const char* variable;
		const char* value;

		MatchItem( const Match& match_, const char* variable_, const char* value_)
			:Match(match_),variable(variable_),value(value_){}
		MatchItem( const MatchItem& o)
			:Match(o),variable(o.variable),value(o.value){}

		MatchItem& operator = (const MatchItem& o) {Match::operator=(o); variable=o.variable; value=o.value; return *this;}
	};

	struct MatchResult
	{
		Match match;
		std::vector<MatchItem> items;

		MatchResult()
			:match(),items(){}
		MatchResult( const Match& match_, const std::vector<MatchItem>& items_=std::vector<MatchItem>())
			:match(match_),items(items_){}
		MatchResult( const MatchResult& o)
			:match(o.match),items(o.items){}
		MatchResult& operator = (const MatchResult& o) {match=o.match; items=o.items; return *this;}
	};

	static void joinResult( MatchResult& result, const MatchResult& aresult);
	bool findFirstMatch( MatchResult& result, unsigned int id, int inputiter, unsigned int maxordlen, bool imm, bool seq) const;
	bool matchCombined( MatchResult& result, unsigned int structid, const unsigned int* idar, int size, int inputiter, unsigned int maxordlen, bool imm, bool seq, unsigned int cardinality) const;
	bool matchShortest( MatchResult& result, std::vector<unsigned int>::const_iterator begin, const std::vector<unsigned int>::const_iterator& end, int inputiter) const;
	bool matchShortest( MatchResult& result, unsigned int const* begin, const unsigned int* end, int inputiter) const;
	bool matchAll( MatchResult& result, std::vector<unsigned int>::const_iterator begin, const std::vector<unsigned int>::const_iterator& end, int inputiter, int cardinality) const;
	bool matchAll( MatchResult& result, unsigned int const* begin, const unsigned int* end, int inputiter, int cardinality) const;
	bool matchItem( MatchResult& result, unsigned int id, int inputiter) const;
	void evalPattern( std::vector<analyzer::PatternMatcherResult>& res, const TestPatternMatcherInstance::Pattern& pattern);

private:
	ErrorBufferInterface* m_errorhnd;
	DebugTraceContextInterface* m_debugtrace;
	DebugTraceContextInterface* m_debugtrace_proc;
	const TestPatternMatcherInstance* m_instance;
	PatternResultFormatContext m_resultFormatContext;
	std::vector<analyzer::PatternLexem> m_inputar;
};

}//namespace
#endif

