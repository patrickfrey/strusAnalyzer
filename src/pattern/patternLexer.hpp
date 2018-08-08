/*
 * Copyright (c) 2018 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Test implementation of the lexer interface for pattern matching
/// \file "patternLexer.hpp"
#ifndef _STRUS_ANALYZER_TEST_PATTERN_LEXER_IMPL_HPP_INCLUDED
#define _STRUS_ANALYZER_TEST_PATTERN_LEXER_IMPL_HPP_INCLUDED
#include "strus/patternLexerInterface.hpp"
#include "strus/patternLexerInstanceInterface.hpp"
#include "strus/patternLexerContextInterface.hpp"
#include "strus/analyzer/patternLexem.hpp"
#include "strus/analyzer/positionBind.hpp"
#include "strus/analyzer/functionView.hpp"
#include "strus/base/regex.hpp"
#include <stdexcept>
#include <map>
#include <set>
#include <string>

namespace strus {

///\brief Forward declaration
class ErrorBufferInterface;
///\brief Forward declaration
class DebugTraceContextInterface;

/// \brief Test implementation of the lexer interface for pattern matching
class TestPatternLexer
	:public PatternLexerInterface
{
public:
	TestPatternLexer( ErrorBufferInterface* errorhnd_)
		:m_errorhnd(errorhnd_){}
	virtual ~TestPatternLexer(){}

	virtual std::vector<std::string> getCompileOptionNames() const	{return std::vector<std::string>();}
	virtual PatternLexerInstanceInterface* createInstance() const;
	virtual const char* getDescription() const			{return "test pattern lexer";}

private:
	ErrorBufferInterface* m_errorhnd;
};


/// \brief Test implementation of the lexer instance interface for pattern matching
class TestPatternLexerInstance
	:public PatternLexerInstanceInterface
{
public:
	TestPatternLexerInstance( ErrorBufferInterface* errorhnd_);

	virtual ~TestPatternLexerInstance();
	virtual void defineOption( const std::string& name, double value);
	virtual void defineLexemName( unsigned int id, const std::string& name);
	virtual void defineLexem(
			unsigned int id,
			const std::string& expression,
			unsigned int resultIndex,
			unsigned int level,
			analyzer::PositionBind posbind);

	virtual void defineSymbol(
			unsigned int id,
			unsigned int lexemid,
			const std::string& name);

	virtual unsigned int getSymbol(
			unsigned int lexemid,
			const std::string& name) const;

	virtual const char* getLexemName( unsigned int id) const;

	virtual bool compile();

	virtual PatternLexerContextInterface* createContext() const;

	virtual analyzer::FunctionView view() const;

private:
	typedef std::map<std::string,int> SymbolTable;

	struct Expression
	{
		unsigned int id;
		unsigned int level;
		RegexSearch* regex;
		analyzer::PositionBind posbind;

		Expression( unsigned int id_, unsigned int level_, RegexSearch* regex_, analyzer::PositionBind posbind_)
			:id(id_),level(level_),regex(regex_),posbind(posbind_){}
		Expression( const Expression& o)
			:id(o.id),level(o.level),regex(o.regex),posbind(o.posbind){}
	};

	friend class TestPatternLexerContext;

	ErrorBufferInterface* m_errorhnd;
	DebugTraceContextInterface* m_debugtrace;
	std::map<unsigned int,std::string> m_lexemNameMap;
	std::map<unsigned int,SymbolTable> m_symmap;
	std::vector<Expression> m_expressions;
	bool m_done;
};


/// \brief Test implementation of the lexer context interface for pattern matching
class TestPatternLexerContext
	:public PatternLexerContextInterface
{
public:
	TestPatternLexerContext( ErrorBufferInterface* errorhnd_, const TestPatternLexerInstance* instance_);
	virtual ~TestPatternLexerContext();

	virtual std::vector<analyzer::PatternLexem> match( const char* src, std::size_t srclen);

	virtual void reset();

private:
	ErrorBufferInterface* m_errorhnd;
	DebugTraceContextInterface* m_debugtrace;
	DebugTraceContextInterface* m_debugtrace_proc;
	const TestPatternLexerInstance* m_instance;
};

}//namespace
#endif

