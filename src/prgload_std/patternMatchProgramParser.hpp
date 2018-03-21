/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Implements the parser for a pattern match program
/// \file patternMatchProgramParser.hpp
#ifndef _STRUS_ANALYZER_PROGRAM_PATTERN_MATCH_PROGRAM_PARSER_INCLUDED
#define _STRUS_ANALYZER_PROGRAM_PATTERN_MATCH_PROGRAM_PARSER_INCLUDED
#include "strus/patternMatcherInstanceInterface.hpp"
#include "strus/patternLexerInstanceInterface.hpp"
#include "strus/patternTermFeederInstanceInterface.hpp"
#include "strus/reference.hpp"
#include "strus/base/stdint.h"
#include "strus/base/symbolTable.hpp"
#include <string>
#include <vector>
#include <map>
#include <set>

/// \brief strus toplevel namespace
namespace strus {

/// \brief Forward declaration
class PatternLexerInstanceInterface;
/// \brief Forward declaration
class PatternTermFeederInstanceInterface;
/// \brief Forward declaration
class PatternMatcherInstanceInterface;
/// \brief Forward declaration
class ErrorBufferInterface;
/// \brief Forward declaration
class PatternMatcherProgram;
/// \brief Forward declaration
class ProgramLexer;

enum {MaxPatternTermNameId=(1<<24)};

class PatternMatcherProgramParser
{
public:
	/// \brief Constructor
	PatternMatcherProgramParser(
			PatternLexerInstanceInterface* crm,
			PatternMatcherInstanceInterface* tpm,
			ErrorBufferInterface* errorhnd_);

	/// \brief Constructor
	PatternMatcherProgramParser(
			PatternTermFeederInstanceInterface* tfm,
			PatternMatcherInstanceInterface* tpm,
			ErrorBufferInterface* errorhnd_);

	bool load( const std::string& source);
	bool compile();
	const std::vector<std::string>& warnings() const	{return m_warnings;}

	bool hasMatcherRules() const
	{
		return m_patternNameSymbolTab.size() > 0;
	}

public:
	struct SubExpressionInfo
	{
		explicit SubExpressionInfo( unsigned int minrange_=0,unsigned int maxrange_=0)
			:minrange(minrange_),maxrange(maxrange_){}
		SubExpressionInfo( const SubExpressionInfo& o)
			:minrange(o.minrange),maxrange(o.maxrange){}

		unsigned int minrange;
		unsigned int maxrange;
	};

	uint32_t getOrCreateSymbol( unsigned int regexid, const std::string& name);
	const char* getSymbolRegexId( unsigned int id) const;
	unsigned int defineAnalyzerTermType( const std::string& type);
	unsigned int getAnalyzerTermType( const std::string& type) const;
	void loadExpressionNode( ProgramLexer& lexer, const std::string& name, SubExpressionInfo& exprinfo);
	void loadExpression( ProgramLexer& lexer, SubExpressionInfo& exprinfo);

private:
	ErrorBufferInterface* m_errorhnd;
	PatternMatcherInstanceInterface* m_patternMatcher;
	PatternLexerInstanceInterface* m_patternLexer;
	PatternTermFeederInstanceInterface* m_patternTermFeeder;
	SymbolTable m_regexNameSymbolTab;
	SymbolTable m_patternNameSymbolTab;
	SymbolTable m_lexemSymbolTab;
	std::map<uint32_t,unsigned int> m_patternLengthMap;
	std::vector<uint32_t> m_symbolRegexIdList;
	std::set<uint32_t> m_unresolvedPatternNameSet;
	std::vector<std::string> m_warnings;
};

}//namespace
#endif

