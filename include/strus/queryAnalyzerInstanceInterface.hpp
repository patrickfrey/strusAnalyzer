/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Parameterizable query analyzer interface
/// \file queryAnalyzerInstanceInterface.hpp
#ifndef _STRUS_ANALYZER_QUERY_ANALYZER_INTERFACE_HPP_INCLUDED
#define _STRUS_ANALYZER_QUERY_ANALYZER_INTERFACE_HPP_INCLUDED
#include "strus/normalizerFunctionInstanceInterface.hpp"
#include "strus/tokenizerFunctionInstanceInterface.hpp"
#include "strus/analyzer/queryAnalyzerView.hpp"
#include <vector>
#include <string>

/// \brief strus toplevel namespace
namespace strus
{
/// \brief Forward declaration
class QueryAnalyzerContextInterface;
/// \brief Forward declaration
class PatternTermFeederInstanceInterface;
/// \brief Forward declaration
class PatternMatcherInstanceInterface;
/// \brief Forward declaration
class PatternLexerInstanceInterface;

/// \brief Defines a program for analyzing chunks of a query
class QueryAnalyzerInstanceInterface
{
public:
	/// \brief Destructor
	virtual ~QueryAnalyzerInstanceInterface(){}

	/// \brief Declare an element to be retrieved from the search index
	/// \param[in] termtype term type name of the feature
	/// \param[in] fieldtype type of the field of this element in the query
	/// \param[in] tokenizer tokenizer (ownership passed to this) to use for this feature
	/// \param[in] normalizers list of normalizers (element ownership passed to this) to use for this feature
	/// \param[in] priority element priority analyzer element with lower priority are ousted if they are completely covered by elements with higher priority
	virtual void addElement(
			const std::string& termtype,
			const std::string& fieldtype,
			TokenizerFunctionInstanceInterface* tokenizer,
			const std::vector<NormalizerFunctionInstanceInterface*>& normalizers,
			int priority)=0;

	/// \brief Declare an element to be used as lexem by post processing pattern matching but not put into the result of query analysis
	/// \param[in] termtype term type name of the lexem to be feed to the pattern matching
	/// \param[in] fieldtype type of the field of this element in the query
	/// \param[in] tokenizer tokenizer (ownership passed to this) to use for this feature
	/// \param[in] normalizers list of normalizers (element ownership passed to this) to use for this feature
	/// \param[in] priority element priority analyzer element with lower priority are ousted if they are completely covered by elements with higher priority
	virtual void addPatternLexem(
			const std::string& termtype,
			const std::string& fieldtype,
			TokenizerFunctionInstanceInterface* tokenizer,
			const std::vector<NormalizerFunctionInstanceInterface*>& normalizers,
			int priority)=0;

	/// \brief Declare a pattern matcher on the query features after other query analysis
	/// \param[in] patternTypeName name of the type to assign to the pattern matching results
	/// \param[in] matcher pattern matcher compiled (ownership passed to this) 
	/// \param[in] feeder feeder that maps document analysis term to pattern lexems as input of the matcher (ownership passed to this) 
	virtual void definePatternMatcherPostProc(
			const std::string& patternTypeName,
			PatternMatcherInstanceInterface* matcher,
			PatternTermFeederInstanceInterface* feeder)=0;

	/// \brief Declare a pattern matcher on the query features after other query analysis
	/// \param[in] patternTypeName name of the type to assign to the pattern matching results
	/// \param[in] matcher pattern matcher compiled (ownership passed to this) 
	/// \param[in] lexer lexer that tokenizes a document segment as input of pattern matching (ownership passed to this) 
	/// \param[in] selectexpr list of selection expressions as input of the pattern matching
	virtual void definePatternMatcherPreProc(
			const std::string& patternTypeName,
			PatternMatcherInstanceInterface* matcher,
			PatternLexerInstanceInterface* lexer,
			const std::vector<std::string>& selectexpr)=0;

	/// \brief Declare a feature to be searched for derived from a pattern matcher result item
	/// \param[in] type type name of the feature
	/// \param[in] patternTypeName type name of the pattern match result or result item
	/// \param[in] normalizers list of normalizers (element ownership passed to this) to use for this feature
	/// \param[in] options (only for pre processing patterns) options that stear the document analysis result, e.g. influence the assingment of document position of terms produced
	/// \param[in] priority element priority analyzer element with lower priority are ousted if they are completely covered by elements with higher priority
	virtual void addElementFromPatternMatch(
			const std::string& type,
			const std::string& patternTypeName,
			const std::vector<NormalizerFunctionInstanceInterface*>& normalizers,
			int priority)=0;

	/// \brief Get the query term types declared in order of appearance in declarations
	/// return the query field types
	virtual std::vector<std::string> queryTermTypes() const=0;

	/// \brief Get the query field types declared in order of appearance in declarations
	/// return the query field types
	virtual std::vector<std::string> queryFieldTypes() const=0;

	/// \brief Create the context used for analyzing a query
	/// \return the query analyzer context (with ownership)
	virtual QueryAnalyzerContextInterface* createContext() const=0;

	/// \brief Return a structure with all definitions for introspection
	/// \return the structure with all definitions for introspection
	virtual analyzer::QueryAnalyzerView view() const=0;
};

}//namespace
#endif

