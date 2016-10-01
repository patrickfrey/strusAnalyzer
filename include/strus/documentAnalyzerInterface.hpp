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
#include "strus/analyzer/term.hpp"
#include "strus/analyzer/attribute.hpp"
#include "strus/analyzer/metaData.hpp"
#include "strus/analyzer/document.hpp"
#include "strus/analyzer/positionBind.hpp"
#include "strus/analyzer/documentClass.hpp"
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


/// \brief Defines a program for analyzing a document, splitting it into normalized terms that can be fed to the strus IR engine
class DocumentAnalyzerInterface
{
public:
	/// \brief Destructor
	virtual ~DocumentAnalyzerInterface(){}

	/// \class FeatureOptions
	/// \brief Options to stear the creation of terms in the analyzer
	class FeatureOptions
	{
	public:
		/// \brief Default constructor
		FeatureOptions()
			:m_opt(0){}
		/// \brief Copy constructor
		FeatureOptions( const FeatureOptions& o)
			:m_opt(o.m_opt){}
		/// \brief Constructor
		FeatureOptions( unsigned int opt_)
			:m_opt(opt_){}

		/// \brief Get the PositionBind value set
		analyzer::PositionBind positionBind() const		{return (analyzer::PositionBind)(m_opt & 0x3);}

		/// \brief Define the PositionBind value
		void definePositionBind( analyzer::PositionBind b)	{m_opt &= ~0x3; m_opt |= (unsigned int)b;}

		/// \brief Get the options transacription as integer
		unsigned int opt() const				{return m_opt;}

	private:
		unsigned int m_opt;
	};

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
			const FeatureOptions& options=FeatureOptions())=0;

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
			const FeatureOptions& options=FeatureOptions())=0;

	/// \brief Declare a feature to be put into the meta data table used for restrictions, weighting and summarization.
	/// \param[in] fieldname name of the field in the meta data table this feature is written to
	/// \param[in] selectexpr an expression that decribes what elements are taken from a document for this feature (tag selection in abbreviated syntax of XPath)
	/// \param[in] tokenizer tokenizer (ownership passed to this) to use for this feature
	/// \param[in] normalizers list of normalizers (ownership of elements passed to this) to use for this feature
	/// \remark The field in the meta data table must exist before this function is called
	virtual void defineMetaData(
			const std::string& fieldname,
			const std::string& selectexpr,
			TokenizerFunctionInstanceInterface* tokenizer,
			const std::vector<NormalizerFunctionInstanceInterface*>& normalizers)=0;

	/// \brief Declare some collected statistics of the document to be put into the meta data table used for restrictions, weighting and summarization.
	/// \param[in] fieldname name of the field in the meta data table this feature is written to
	/// \param[in] statfunc function (ownership passed to this) that decribes how the value to be inserted is calculated from a document
	/// \remark The field in the meta data table must exist before this function is called
	virtual void defineAggregatedMetaData(
			const std::string& fieldname,
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
	/// \return the analyzer context (ownership to caller)
	virtual DocumentAnalyzerContextInterface* createContext(
			const analyzer::DocumentClass& dclass) const=0;
};

}//namespace
#endif

