/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Interface for a parametrizable document analyzer instance
/// \file documentAnalyzerInstanceInterface.hpp
#ifndef _STRUS_ANALYZER_DOCUMENT_ANALYZER_INTERFACE_HPP_INCLUDED
#define _STRUS_ANALYZER_DOCUMENT_ANALYZER_INTERFACE_HPP_INCLUDED
#include "strus/analyzer/document.hpp"
#include "strus/analyzer/positionBind.hpp"
#include "strus/analyzer/documentClass.hpp"
#include "strus/analyzer/featureOptions.hpp"
#include "strus/analyzer/segmenterOptions.hpp"
#include "strus/structView.hpp"
#include <vector>
#include <string>
#include <cstring>

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

/// \brief Defines a program for analyzing a document, splitting it into normalized terms that can be fed to the strus IR engine
class DocumentAnalyzerInstanceInterface
{
public:
	/// \brief Destructor
	virtual ~DocumentAnalyzerInstanceInterface(){}

	/// \brief Declare a feature to be put into the search index
	/// \param[in] type type name of the feature
	/// \param[in] selectexpr an expression that decribes what elements are taken from a document for this feature (tag selection in abbreviated syntax of XPath)
	/// \param[in] tokenizer tokenizer (ownership passed to this) to use for this feature
	/// \param[in] normalizers list of normalizers (element ownership passed to this) to use for this feature
	/// \param[in] priority element priority analyzer element with lower priority are ousted if they are completely covered by elements with higher priority
	/// \param[in] options options that stear the document analysis result (e.g. influence the assingment of document position of terms produced)
	virtual void addSearchIndexFeature(
			const std::string& type,
			const std::string& selectexpr,
			TokenizerFunctionInstanceInterface* tokenizer,
			const std::vector<NormalizerFunctionInstanceInterface*>& normalizers,
			int priority,
			const analyzer::FeatureOptions& options)=0;

	/// \brief Declare a feature to be put into the forward index used for summarization extraction.
	/// \param[in] type type name of the feature
	/// \param[in] selectexpr an expression that decribes what elements are taken from a document for this feature (tag selection in abbreviated syntax of XPath)
	/// \param[in] tokenizer tokenizer (ownership passed to this) to use for this feature
	/// \param[in] normalizers list of normalizers (ownership of elements passed to this) to use for this feature
	/// \param[in] priority element priority analyzer element with lower priority are ousted if they are completely covered by elements with higher priority
	/// \param[in] options options that stear the document analysis result (e.g. influence the assingment of document position of terms produced)
	virtual void addForwardIndexFeature(
			const std::string& type,
			const std::string& selectexpr,
			TokenizerFunctionInstanceInterface* tokenizer,
			const std::vector<NormalizerFunctionInstanceInterface*>& normalizers,
			int priority,
			const analyzer::FeatureOptions& options)=0;

	/// \brief Declare a field, a positional range of elements and assign a classifying name and a contextual unique identifier to it.
	/// \param[in] fieldTypeName type name of the field, for addressing it in structure definitions
	/// \param[in] scopeexpr defining the scope of the field identifiers and the parent expression of select and id (tag selection in abbreviated syntax of XPath)
	/// \param[in] selectexpr select expression relative to scope expression selecting the positional range of the field (tag selection in abbreviated syntax of XPath)
	/// \param[in] keyexpr select expression relative to scope expression selecting the identifier of the field within its scope (tag selection in abbreviated syntax of XPath)
	virtual void addSearchIndexField(
			const std::string& fieldTypeName,
			const std::string& scopeexpr,
			const std::string& selectexpr,
			const std::string& keyexpr)=0;

	/// \brief Classification of structures declared as relation of two fields
	enum StructureType
	{
		StructureHierarchical,		///< Header is embedding content or vice versa
		StructureHeading,		///< Content is following header, contents are attached to header until new header appears
		StructureFooter			///< Header is following content, open contents are attached to header when the next header appears
	};
	static const char* structureTypeName( StructureType t)
	{
		const char* ar[] = {"hierarchical","heading","footer",0};
		return ar[t];
	}
	static bool structureTypeFromName( StructureType& type, const char* name)
	{
		type = (StructureType)0;
		char const* typestr = structureTypeName( type);
		for (; typestr; typestr = structureTypeName( type=(StructureType)(type+1)))
		{
			if (0==std::strcmp( name, typestr)) return true;
		}
		return false;
	}

	/// \brief Declare a structure as unidirectional relation of two fields
	/// \note The source of the relation is called header and the sink is called content
	/// \param[in] structureTypeName type name of the structure, for addressing it in the query
	/// \param[in] headerFieldName header field name of the structure
	/// \param[in] contentFieldName content field name of the structure
	/// \param[in] structureType type of the structure 
	virtual void addSearchIndexStructure(
			const std::string& structureTypeName,
			const std::string& headerFieldName,
			const std::string& contentFieldName,
			const StructureType& structureType)=0;

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
	/// \param[in] priority element priority analyzer element with lower priority are ousted if they are completely covered by elements with higher priority
	virtual void addPatternLexem(
			const std::string& termtype,
			const std::string& selectexpr,
			TokenizerFunctionInstanceInterface* tokenizer,
			const std::vector<NormalizerFunctionInstanceInterface*>& normalizers,
			int priority)=0;

	/// \brief Declare a pattern matcher on the document terms of pattern lexems after normalization
	/// \param[in] patternName name of the type to assign to the pattern matching results
	/// \param[in] matcher pattern matcher compiled (ownership passed to this) 
	/// \param[in] feeder feeder that maps document analysis term to pattern lexems as input of the matcher (ownership passed to this) 
	virtual void defineTokenPatternMatcher(
			const std::string& patternName,
			PatternMatcherInstanceInterface* matcher,
			PatternTermFeederInstanceInterface* feeder)=0;

	/// \brief Declare a pattern matcher on the document features after other document analysis
	/// \param[in] patternName name of the type to assign to the pattern matching results
	/// \param[in] matcher pattern matcher compiled (ownership passed to this) 
	/// \param[in] lexer lexer that tokenizes a document segment as input of pattern matching (ownership passed to this) 
	/// \param[in] selectexpr list of selection expressions as input of the pattern matching
	virtual void defineContentPatternMatcher(
			const std::string& patternName,
			PatternMatcherInstanceInterface* matcher,
			PatternLexerInstanceInterface* lexer,
			const std::vector<std::string>& selectexpr)=0;

	/// \brief Declare a feature to be put into the search index derived from a pattern matcher result item
	/// \param[in] type type name of the feature
	/// \param[in] patternName type name of the pattern match result or result item
	/// \param[in] normalizers list of normalizers (element ownership passed to this) to use for this feature
	/// \param[in] priority element priority analyzer element with lower priority are ousted if they are completely covered by elements with higher priority
	/// \param[in] options (only for pre processing patterns) options that stear the document analysis result, e.g. influence the assingment of document position of terms produced
	virtual void addSearchIndexFeatureFromPatternMatch(
			const std::string& type,
			const std::string& patternName,
			const std::vector<NormalizerFunctionInstanceInterface*>& normalizers,
			int priority,
			const analyzer::FeatureOptions& options)=0;

	/// \brief Declare a feature to be put into the forward index derived from a pattern matcher result item
	/// \param[in] type type name of the feature
	/// \param[in] patternName type name of the pattern match result or result item
	/// \param[in] normalizers list of normalizers (element ownership passed to this) to use for this feature
	/// \param[in] priority element priority analyzer element with lower priority are ousted if they are completely covered by elements with higher priority
	/// \param[in] options (only for pre processing patterns) options that stear the document analysis result, e.g. influence the assingment of document position of terms produced
	virtual void addForwardIndexFeatureFromPatternMatch(
			const std::string& type,
			const std::string& patternName,
			const std::vector<NormalizerFunctionInstanceInterface*>& normalizers,
			int priority,
			const analyzer::FeatureOptions& options)=0;

	/// \brief Declare a feature to be put into the meta data table for restrictions, weighting and summarization, derived from a pattern matcher result item
	/// \param[in] metaname name of the column in the meta data table this feature is written to
	/// \param[in] patternName type name of the pattern match result or result item
	/// \param[in] normalizers list of normalizers (element ownership passed to this) to use for this feature
	/// \remark The field in the meta data table must exist before this function is called
	virtual void defineMetaDataFromPatternMatch(
			const std::string& metaname,
			const std::string& patternName,
			const std::vector<NormalizerFunctionInstanceInterface*>& normalizers)=0;

	/// \brief Declare a feature to be defined as document attribute used for summarization, derived from a pattern matcher result item
	/// \param[in] attribname name of the document attribute assigned
	/// \param[in] patternName type name of the pattern match result or result item
	/// \param[in] normalizers list of normalizers (element ownership passed to this) to use for this feature
	/// \remark The field in the meta data table must exist before this function is called
	virtual void defineAttributeFromPatternMatch(
			const std::string& attribname,
			const std::string& patternName,
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
	virtual StructView view() const=0;
};

}//namespace
#endif

