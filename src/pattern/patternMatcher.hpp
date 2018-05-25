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
#include "strus/reference.hpp"
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

		Pattern( unsigned int id_, const std::string& name_)
			:id(id_),name(name_){}
		Pattern( const Pattern& o)
			:id(o.id),name(o.name){}
	};
	typedef std::map<unsigned int,const char*> ExpressionVariableMap;
	typedef std::map<unsigned int,const PatternResultFormat*> ExpressionResultFormatMap;

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

	const PatternResultFormat* getResultFormat( unsigned int id) const;
	const char* getVariableAttached( unsigned int id) const;
	const char* getVariableId( const std::string& name) const;
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
	ExpressionResultFormatMap m_exprfmtmap;
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
		MatchAddress& operator=( const MatchAddress& o)
			{seg=o.seg;pos=o.pos; return *this;}

		bool operator < (const MatchAddress& o) const
		{
			return (seg == o.seg) ? pos < o.pos : seg < o.seg;
		}
		bool operator > (const MatchAddress& o) const
		{
			return (seg == o.seg) ? pos > o.pos : seg > o.seg;
		}
	};

	struct MatchResult
	{
		std::string name;
		std::string value;
		int ordpos;
		int ordlen;
		MatchAddress start;
		MatchAddress end;
		std::vector<strus::Reference<MatchResult> > items;

		MatchResult()
			:name(),value(),ordpos(-1),ordlen(-1),start(),end(),items(){}
		MatchResult( const std::string& name_, const std::string& value_, int ordpos_, int ordlen_, const MatchAddress& start_, const MatchAddress& end_, std::vector<strus::Reference<MatchResult> > items_)
			:name(),value(),ordpos(ordpos_),ordlen(ordlen_),start(start_),end(end_),items(items_){}
		MatchResult( const std::string& name_, const std::string& value_, int ordpos_, int ordlen_, const MatchAddress& start_, const MatchAddress& end_)
			:name(),value(),ordpos(ordpos_),ordlen(ordlen_),start(start_),end(end_),items(){}
		MatchResult( const MatchResult& o)
			:name(o.name),value(o.value),ordpos(o.ordpos),ordlen(o.ordlen),start(o.start),end(o.end),items(o.items){}
		MatchResult& operator = (const MatchResult& o) {name=o.name;value=o.value;ordpos=o.ordpos;ordlen=o.ordlen;start=o.start;end=o.end;items=o.items; return *this;}

		bool defined() const	{return ordpos > 0;}
	};

	typedef const TestPatternMatcherInstance::Pattern* PatternPtr;

	std::string mapResultValue( unsigned int id, const MatchResult& result);
	const char* allocCharp( const std::string& value);

	void joinResult( MatchResult& result, unsigned int id, const MatchResult& aresult);
	bool findFirstMatch( MatchResult& result, unsigned int id, int inputiter, unsigned int maxordlen, bool imm, bool seq);
	bool matchCombined( MatchResult& result, unsigned int structid, const unsigned int* idar, int idarsize, int inputiter, unsigned int maxordlen, bool imm, bool seq, unsigned int cardinality);
	bool matchShortest( MatchResult& result, std::vector<unsigned int>::const_iterator begin, const std::vector<unsigned int>::const_iterator& end, int inputiter);
	bool matchShortest( MatchResult& result, const unsigned int* begin, const unsigned int* end, int inputiter);
	bool matchAll( MatchResult& result, std::vector<unsigned int>::const_iterator begin, const std::vector<unsigned int>::const_iterator& end, int inputiter, int cardinality);
	bool matchAll( MatchResult& result, const unsigned int* begin, const unsigned int* end, int inputiter, int cardinality);
	bool matchItem( MatchResult& result, unsigned int pattern, int inputiter);

	void evalPattern( std::vector<analyzer::PatternMatcherResult>& res, const TestPatternMatcherInstance::Pattern& pattern);

private:
	ErrorBufferInterface* m_errorhnd;
	DebugTraceContextInterface* m_debugtrace;
	DebugTraceContextInterface* m_debugtrace_proc;
	const TestPatternMatcherInstance* m_instance;
	PatternResultFormatContext m_resultFormatContext;
	std::vector<analyzer::PatternLexem> m_inputar;
	std::list<std::string> m_values;
};

}//namespace
#endif

