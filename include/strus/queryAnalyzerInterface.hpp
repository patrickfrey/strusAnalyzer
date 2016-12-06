/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Parameterizable query analyzer interface
/// \file queryAnalyzerInterface.hpp
#ifndef _STRUS_ANALYZER_QUERY_ANALYZER_INTERFACE_HPP_INCLUDED
#define _STRUS_ANALYZER_QUERY_ANALYZER_INTERFACE_HPP_INCLUDED
#include "strus/analyzer/term.hpp"
#include "strus/normalizerFunctionInstanceInterface.hpp"
#include "strus/tokenizerFunctionInstanceInterface.hpp"
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
class QueryAnalyzerInterface
{
public:
	/// \brief Destructor
	virtual ~QueryAnalyzerInterface(){}

	/// \brief Declare an element to be retrieved from the search index
	/// \param[in] termtype term type name of the feature
	/// \param[in] fieldtype type of the field of this element in the query
	/// \param[in] tokenizer tokenizer (ownership passed to this) to use for this feature
	/// \param[in] normalizers list of normalizers (element ownership passed to this) to use for this feature
	virtual void addSearchIndexElement(
			const std::string& termtype,
			const std::string& fieldtype,
			TokenizerFunctionInstanceInterface* tokenizer,
			const std::vector<NormalizerFunctionInstanceInterface*>& normalizers)=0;

	/// \brief Declare a feature to be searched in the meta data table
	/// \param[in] metaname name of the column in the meta data table this feature is searched in
	/// \param[in] fieldtype type of the field of this element in the query
	/// \param[in] tokenizer tokenizer (ownership passed to this) to use for this feature
	/// \param[in] normalizers list of normalizers (ownership of elements passed to this) to use for this feature
	/// \remark The field in the meta data table must exist before this function is called
	virtual void addMetaDataElement(
			const std::string& metaname,
			const std::string& fieldtype,
			TokenizerFunctionInstanceInterface* tokenizer,
			const std::vector<NormalizerFunctionInstanceInterface*>& normalizers)=0;

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
	virtual void addSearchIndexFeatureFromPatternMatch(
			const std::string& type,
			const std::string& patternTypeName,
			const std::vector<NormalizerFunctionInstanceInterface*>& normalizers)=0;

	/// \brief Declare a feature of the meta data table for restrictions, weighting and summarization, derived from a pattern matcher result item
	/// \param[in] metaname name of the column in the meta data table this feature is written to
	/// \param[in] patternTypeName type name of the pattern match result or result item
	/// \param[in] normalizers list of normalizers (element ownership passed to this) to use for this feature
	/// \remark The field in the meta data table must exist before this function is called
	virtual void defineMetaDataFromPatternMatch(
			const std::string& metaname,
			const std::string& patternTypeName,
			const std::vector<NormalizerFunctionInstanceInterface*>& normalizers)=0;
	
	/// \brief Create the context used for analyzing a query
	/// \return the query analyzer context (with ownership)
	virtual QueryAnalyzerContextInterface* createContext() const=0;
};

}//namespace
#endif

