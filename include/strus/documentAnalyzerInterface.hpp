/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Interface for a parametrizable document analyzer instance
/// \file documentAnalyzerInterface.hpp
#ifndef _STRUS_ANALYZER_DOCUMENT_ANALYZER_INTERFACE_HPP_INCLUDED
#define _STRUS_ANALYZER_DOCUMENT_ANALYZER_INTERFACE_HPP_INCLUDED
#include "strus/analyzer/document.hpp"
#include "strus/analyzer/positionBind.hpp"
#include "strus/analyzer/documentClass.hpp"
#include "strus/analyzer/featureOptions.hpp"
#include "strus/analyzer/segmenterOptions.hpp"
#include "strus/analyzer/documentAnalyzerView.hpp"
#include "strus/analyzer/functionView.hpp"
#include <vector>
#include <string>

/// \brief strus toplevel namespace
namespace strus
{

/// \brief Forward declaration
class DocumentAnalyzerContextInterface;
/// \brief Forward declaration
class NormalizerFunctionInstanceInterface;
/// \brief Forward declaration
class TokenizerFunctionInstanceInterface;
/// \brief Forward declaration
class AggregatorFunctionInstanceInterface;
/// \brief Forward declaration
class SegmenterInterface;
/// \brief Forward declaration
class PatternTermFeederInstanceInterface;
/// \brief Forward declaration
class PatternMatcherInstanceInterface;
/// \brief Forward declaration
class PatternLexerInstanceInterface;
/// \brief Forward declaration
class IntrospectionInterface;

/// \brief Defines a program for analyzing a document, splitting it into normalized terms that can be fed to the strus IR engine
class DocumentAnalyzerInterface
{
public:
	/// \brief Destructor
	virtual ~DocumentAnalyzerInterface(){}

	/// \brief Declare a feature to be put into the search index
	/// \param[in] type type name of the feature
	/// \param[in] selectexpr an expression that decribes what elements are taken from a document for this feature (tag selection in abbreviated syntax of XPath)
	/// \param[in] tokenizer tokenizer (ownership passed to this) to use for this feature
	/// \param[in] normalizers list of normalizers (element ownership passed to this) to use for this feature
	/// \param[in] options options that stear the document analysis result (e.g. influence the assingment of document position of terms produced)
	virtual void addSearchIndexFeature(
			const std::string& type,
			const std::string& selectexpr,
			TokenizerFunctionInstanceInterface* tokenizer,
			const std::vector<NormalizerFunctionInstanceInterface*>& normalizers,
			const analyzer::FeatureOptions& options)=0;

	/// \brief Declare a feature to be put into the forward index used for summarization extraction.
	/// \param[in] type type name of the feature
	/// \param[in] selectexpr an expression that decribes what elements are taken from a document for this feature (tag selection in abbreviated syntax of XPath)
	/// \param[in] tokenizer tokenizer (ownership passed to this) to use for this feature
	/// \param[in] normalizers list of normalizers (ownership of elements passed to this) to use for this feature
	/// \param[in] options options that stear the document analysis result (e.g. influence the assingment of document position of terms produced)
	virtual void addForwardIndexFeature(
			const std::string& type,
			const std::string& selectexpr,
			TokenizerFunctionInstanceInterface* tokenizer,
			const std::vector<NormalizerFunctionInstanceInterface*>& normalizers,
			const analyzer::FeatureOptions& options)=0;

	/// \brief Declare a feature to be put into the meta data table used for restrictions, weighting and summarization.
	/// \param[in] metaname name of the column in the meta data table this feature is written to
	/// \param[in] selectexpr an expression that decribes what elements are taken from a document for this feature (tag selection in abbreviated syntax of XPath)
	/// \param[in] tokenizer tokenizer (ownership passed to this) to use for this feature
	/// \param[in] normalizers list of normalizers (ownership of elements passed to this) to use for this feature
	/// \remark The field in the meta data table must exist before this function is called
	virtual void defineMetaData(
			const std::string& metaname,
			const std::string& selectexpr,
			TokenizerFunctionInstanceInterface* tokenizer,
			const std::vector<NormalizerFunctionInstanceInterface*>& normalizers)=0;

	/// \brief Declare some collected statistics of the document to be put into the meta data table used for restrictions, weighting and summarization.
	/// \param[in] metaname name of the column in the meta data table this feature is written to
	/// \param[in] statfunc function (ownership passed to this) that decribes how the value to be inserted is calculated from a document
	/// \remark The field in the meta data table must exist before this function is called
	virtual void defineAggregatedMetaData(
			const std::string& metaname,
			AggregatorFunctionInstanceInterface* statfunc)=0;

	/// \brief Declare a feature to be defined as document attribute used for summarization (document title, document id, etc.)
	/// \param[in] attribname name of the document attribute this feature is written as.
	/// \param[in] selectexpr an expression that decribes what elements are taken from a document for this feature (tag selection in abbreviated syntax of XPath)
	/// \param[in] tokenizer tokenizer (ownership passed to this) to use for this feature
	/// \param[in] normalizers list of normalizers (ownership of elements passed to this) to use for this feature
	/// \remark Attributes must be defined uniquely per document
	virtual void defineAttribute(
			const std::string& attribname,
			const std::string& selectexpr,
			TokenizerFunctionInstanceInterface* tokenizer,
			const std::vector<NormalizerFunctionInstanceInterface*>& normalizers)=0;

	/// \brief Declare a sub document for the handling of multi part documents in an analyzed content
	/// \param[in] selectexpr an expression that defines the content of the sub document declared
	/// \param[in] subDocumentTypeName type name assinged to this sub document
	/// \remark Sub documents are defined as the sections selected by the expression plus some data selected not belonging to any sub document.
	virtual void defineSubDocument(
			const std::string& subDocumentTypeName,
			const std::string& selectexpr)=0;

	/// \brief Declare a sub content of the document that has to be processed with a different segmenter
	/// \param[in] selectexpr an expression that addresses the sub content declared
	/// \param[in] documentClass defines the content type of the sub content
	/// \remark Sub contents have to be defined before (!) any item with a selection expression referring to it.
	virtual void defineSubContent(
			const std::string& selectexpr,
			const analyzer::DocumentClass& documentClass)=0;

	/// \brief Declare an element to be used as lexem by post processing pattern matching but not put into the result of document analysis
	/// \param[in] termtype term type name of the lexem to be feed to the pattern matching
	/// \param[in] selectexpr an expression that decribes what elements are taken from a document for this feature (tag selection in abbreviated syntax of XPath)
	/// \param[in] tokenizer tokenizer (ownership passed to this) to use for this feature
	/// \param[in] normalizers list of normalizers (element ownership passed to this) to use for this feature
	virtual void addPatternLexem(
			const std::string& termtype,
			const std::string& selectexpr,
			TokenizerFunctionInstanceInterface* tokenizer,
			const std::vector<NormalizerFunctionInstanceInterface*>& normalizers)=0;

	/// \brief Declare a pattern matcher on the document features after other document analysis
	/// \param[in] patternTypeName name of the type to assign to the pattern matching results
	/// \param[in] matcher pattern matcher compiled (ownership passed to this) 
	/// \param[in] feeder feeder that maps document analysis term to pattern lexems as input of the matcher (ownership passed to this) 
	virtual void definePatternMatcherPostProc(
			const std::string& patternTypeName,
			PatternMatcherInstanceInterface* matcher,
			PatternTermFeederInstanceInterface* feeder)=0;

	/// \brief Declare a pattern matcher on the document features after other document analysis
	/// \param[in] patternTypeName name of the type to assign to the pattern matching results
	/// \param[in] matcher pattern matcher compiled (ownership passed to this) 
	/// \param[in] lexer lexer that tokenizes a document segment as input of pattern matching (ownership passed to this) 
	/// \param[in] selectexpr list of selection expressions as input of the pattern matching
	virtual void definePatternMatcherPreProc(
			const std::string& patternTypeName,
			PatternMatcherInstanceInterface* matcher,
			PatternLexerInstanceInterface* lexer,
			const std::vector<std::string>& selectexpr)=0;

	/// \brief Declare a feature to be put into the search index derived from a pattern matcher result item
	/// \param[in] type type name of the feature
	/// \param[in] patternTypeName type name of the pattern match result or result item
	/// \param[in] normalizers list of normalizers (element ownership passed to this) to use for this feature
	/// \param[in] options (only for pre processing patterns) options that stear the document analysis result, e.g. influence the assingment of document position of terms produced
	virtual void addSearchIndexFeatureFromPatternMatch(
			const std::string& type,
			const std::string& patternTypeName,
			const std::vector<NormalizerFunctionInstanceInterface*>& normalizers,
			const analyzer::FeatureOptions& options)=0;

	/// \brief Declare a feature to be put into the forward index derived from a pattern matcher result item
	/// \param[in] type type name of the feature
	/// \param[in] patternTypeName type name of the pattern match result or result item
	/// \param[in] normalizers list of normalizers (element ownership passed to this) to use for this feature
	/// \param[in] options (only for pre processing patterns) options that stear the document analysis result, e.g. influence the assingment of document position of terms produced
	virtual void addForwardIndexFeatureFromPatternMatch(
			const std::string& type,
			const std::string& patternTypeName,
			const std::vector<NormalizerFunctionInstanceInterface*>& normalizers,
			const analyzer::FeatureOptions& options)=0;

	/// \brief Declare a feature to be put into the meta data table for restrictions, weighting and summarization, derived from a pattern matcher result item
	/// \param[in] metaname name of the column in the meta data table this feature is written to
	/// \param[in] patternTypeName type name of the pattern match result or result item
	/// \param[in] normalizers list of normalizers (element ownership passed to this) to use for this feature
	/// \remark The field in the meta data table must exist before this function is called
	virtual void defineMetaDataFromPatternMatch(
			const std::string& metaname,
			const std::string& patternTypeName,
			const std::vector<NormalizerFunctionInstanceInterface*>& normalizers)=0;

	/// \brief Declare a feature to be defined as document attribute used for summarization, derived from a pattern matcher result item
	/// \param[in] attribname name of the document attribute assigned
	/// \param[in] patternTypeName type name of the pattern match result or result item
	/// \param[in] normalizers list of normalizers (element ownership passed to this) to use for this feature
	/// \remark The field in the meta data table must exist before this function is called
	virtual void defineAttributeFromPatternMatch(
			const std::string& attribname,
			const std::string& patternTypeName,
			const std::vector<NormalizerFunctionInstanceInterface*>& normalizers)=0;

	/// \brief Segment and tokenize a document, assign types to tokens and metadata and normalize their values
	/// \param[in] content document content string to analyze
	/// \param[in] dclass description of the content type and encoding to process
	/// \return the analyzed document
	/// \remark Do not use this function in case of a multipart document (defined with 'defineSubDocument(const std::string&,const std::string&)') because you get only one sub document analyzed. Use the interface created with 'createDocumentAnalyzerContext(std::istream&)const' instead.
	virtual analyzer::Document analyze(
			const std::string& content,
			const analyzer::DocumentClass& dclass) const=0;

	/// \brief Create the context used for analyzing multipart or very big documents
	/// \param[in] dclass description of the content type and encoding to process
	/// \return the document analyzer context (with ownership)
	virtual DocumentAnalyzerContextInterface* createContext(
			const analyzer::DocumentClass& dclass) const=0;

	/// \brief Return a structure with all definitions for introspection
	/// \return the structure with all definitions for introspection
	virtual analyzer::DocumentAnalyzerView view() const=0;

	/// \brief Create an interface for introspection
	/// \return the introspection interface (with ownership)
	virtual IntrospectionInterface* createIntrospection() const=0;
};

}//namespace
#endif

