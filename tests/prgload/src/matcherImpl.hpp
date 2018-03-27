/*
 * Copyright (c) 2018 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Test implementation of the lexer interface for pattern matching
/// \file "lexerImpl.hpp"
#ifndef _STRUS_ANALYZER_TEST_MATCHER_IMPL_HPP_INCLUDED
#define _STRUS_ANALYZER_TEST_MATCHER_IMPL_HPP_INCLUDED
#include "strus/patternMatcherInterface.hpp"
#include "strus/patternMatcherInstanceInterface.hpp"
#include "strus/patternMatcherContextInterface.hpp"
#include "strus/errorBufferInterface.hpp"
#include "strus/debugTraceInterface.hpp"
#include "strus/base/dynamic_bitset.hpp"
#include <stdexcept>
#include <map>
#include <set>
#include <algorithm>

#define STRUS_COMPONENT_NAME "pattern"

/// \brief Test implementation of the matcher interface for pattern matching
class TestPatternMatcher
	:public strus::PatternMatcherInterface
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
	:public strus::PatternMatcherInstanceInterface
{
public:
	TestPatternMatcherInstance( ErrorBufferInterface* errorhnd_)
		:m_errorhnd(errorhnd_),m_dbgtrace(0),m_patternar(),m_patternrefar(),m_expressionar(),m_variablear(),m_operandsar(),m_stk(),m_done(false)
	{
		strus::DebugTraceInterface* dt = m_errorhnd->debugTrace();
		m_dbgtrace = dt ? dt->createTraceContext( STRUS_COMPONENT_NAME) : NULL;
	}

	virtual ~TestPatternMatcherInstance(){}

	virtual void defineOption( const std::string& name, double value)
	{
		if (m_dbgtrace) m_dbgtrace->event( "option", "%s %f", name.c_str(), value);
		if (m_done) throw std::logic_error("illegal call");
		throw std::runtime_error("unknonw option passed to pattern lexer");
	}

	virtual void defineTermFrequency( unsigned int termid, double df)
	{
		if (m_dbgtrace) m_dbgtrace->event( "df", "%d %f", termid, df);
	}

	virtual void pushTerm( unsigned int termid)
	{
		if (m_dbgtrace) m_dbgtrace->event( "push", "term %d", termid);
		if (termid >= MaxId) throw std::runtime_error("illegal term id pushed");
		m_stk.push_back( termid);
	}

	virtual void pushExpression(
			JoinOperation operation,
			std::size_t argc, unsigned int range, unsigned int cardinality)
	{
		if (m_dbgtrace) m_dbgtrace->event( "push", "expression %s %d %u %u", joinOperationName(operation), (int)argc, range, cardinality);
		if (m_stk.size() < argc) throw std::runtime_error("illegal operation");
		if (m_expressionar.size() > MaxId) throw std::runtime_error("too many expressions pushed");

		int operandsidx = m_operandsar.size();
		m_operandsar.insert( m_operandsar.end(), m_stk.end() - argc, m_stk.end());
		m_stk.resize( m_stk.size() - argc);
		m_stk.push_back( m_expressionar.size() + ExpressionIdOfs);
		m_expressionar.push_back( Expression( operation, argc, range, cardinality, operandsidx));
	}

	virtual void pushPattern( const std::string& name)
	{
		if (m_dbgtrace) m_dbgtrace->event( "push", "pattern %s", name.c_str());
		if (m_stk.empty()) throw std::runtime_error("illegal operation");
		if (m_patternar.size() > MaxId) throw std::runtime_error("too many expressions pushed");

		m_stk.push_back( m_patternrefar.size() + PatternIdOfs);
		m_patternrefar.push_back( name);
	}

	virtual void attachVariable( const std::string& name)
	{
		if (m_dbgtrace) m_dbgtrace->event( "attach", "variable %s", name.c_str());
		if (m_stk.empty()) throw std::runtime_error("illegal operation");
		m_variablear.push_back( Variable( m_stk.back(), name));
	}

	virtual void definePattern( const std::string& name, bool visible)
	{
		if (m_dbgtrace) m_dbgtrace->event( "pattern", "%s %s", name.c_str(), visible?"public":"private");
		m_patternar.push_back( Pattern( m_stk.back(), name));
		m_stk.pop_back();
	}

	virtual bool compile()
	{
		if (m_dbgtrace) m_dbgtrace->event( "compile", "");
		if (m_done) throw std::logic_error("illegal call");
		m_done = true;
		return true;
	}

	virtual PatternMatcherContextInterface* createContext() const;

private:
	friend class TestPatternMatcherContext;

	enum {
		MaxId=(1<<20),
		PatternIdOfs=(1<<28),
		ExpressionIdOfs=(1<<29)
	};
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
	struct Variable
	{
		unsigned int id;
		std::string name;

		Variable( unsigned int id_, const std::string& name_)
			:id(id_),name(name_){}
		Variable( const Variable& o)
			:id(o.id),name(o.name){}
	};
	ErrorBufferInterface* m_errorhnd;
	strus::DebugTraceContextInterface* m_dbgtrace;
	std::vector<Pattern> m_patternar;
	std::vector<std::string> m_patternrefar;
	std::vector<Expression> m_expressionar;
	std::vector<Variable> m_variablear;
	std::vector<unsigned int> m_operandsar;
	std::vector<unsigned int> m_stk;
	bool m_done;
};

/// \brief Test implementation of the matcher context interface for pattern matching
class TestPatternMatcherContext
	:public strus::PatternMatcherContextInterface
{
public:
	TestPatternMatcherContext( ErrorBufferInterface* errorhnd_, const TestPatternMatcherInstance* instance_)
		:m_errorhnd(errorhnd_),m_dbgtrace(0),m_instance(instance_)
	{
		strus::DebugTraceInterface* dt = m_errorhnd->debugTrace();
		m_dbgtrace = dt ? dt->createTraceContext( STRUS_COMPONENT_NAME) : NULL;
	}

	virtual ~TestPatternMatcherContext(){}

	virtual void putInput( const analyzer::PatternLexem& token);

	virtual std::vector<analyzer::PatternMatcherResult> fetchResults() const;

	virtual analyzer::PatternMatcherStatistics getStatistics() const;

	virtual void reset();

private:
	ErrorBufferInterface* m_errorhnd;
	strus::DebugTraceContextInterface* m_dbgtrace;
	const TestPatternMatcherInstance* m_instance;
};


PatternMatcherContextInterface* TestPatternMatcherInstance::createContext() const
{
	return new TestPatternMatcherContext( m_errorhnd, this);
}

PatternMatcherInstanceInterface* TestPatternMatcher::createInstance() const
{
	return new TestPatternMatcherInstance( m_errorhnd);
}




