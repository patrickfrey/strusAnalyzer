/*
---------------------------------------------------------------------
    The C++ library strus implements basic operations to build
    a search engine for structured search on unstructured data.

    Copyright (C) 2013,2014 Patrick Frey

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

--------------------------------------------------------------------

	The latest version of strus can be found at 'http://github.com/patrickfrey/strus'
	For documentation see 'http://patrickfrey.github.com/strus'

--------------------------------------------------------------------
*/
#ifndef _STRUS_ANALYZER_DOCUMENT_ANALYZER_INTERFACE_HPP_INCLUDED
#define _STRUS_ANALYZER_DOCUMENT_ANALYZER_INTERFACE_HPP_INCLUDED
#include "strus/analyzer/term.hpp"
#include "strus/analyzer/attribute.hpp"
#include "strus/analyzer/metaData.hpp"
#include "strus/analyzer/document.hpp"
#include "strus/normalizerConfig.hpp"
#include "strus/tokenizerConfig.hpp"
#include <vector>
#include <string>

namespace strus
{
/// \brief Forward declaration
class DocumentAnalyzerInstanceInterface;

/// \brief Defines a program for analyzing a document, splitting it into normalized terms that can be fed to the strus IR engine
class DocumentAnalyzerInterface
{
public:
	/// \brief Destructor
	virtual ~DocumentAnalyzerInterface(){}

	/// \enum PositionBind
	/// \brief Determines how document positions are assigned to terms
	/// \remark The motivation is to distinguish content elements from markup.
	enum PositionBind
	{
		BindContent,		///< An element in the document that gets an own position assigned
		BindSuccessor,		///< An element in the document that gets the position of the succeding content element assigned
		BindPredecessor		///< An element in the document that gets the position of the preceding content element assigned
	};
	static const char* positionBindName( PositionBind t)
	{
		static const char* ar[] = {"BindContent","BindSuccessor","BindPredecessor"};
		return ar[t];
	}

	/// \brief Declare a feature to be put into the search index
	/// \param[in] type type name of the feature
	/// \param[in] selectexpr an expression that decribes what elements are taken from a document for this feature (tag selection in abbreviated syntax of XPath)
	/// \param[in] tokenizer selects a tokenizer by name describing how text chunks are tokenized
	/// \param[in] normalizer selects a normalizer by name describing how tokens are normalized
	/// \param[in] positionBind influences the assingment of document position of terms produced
	virtual void addSearchIndexFeature(
			const std::string& type,
			const std::string& selectexpr,
			const TokenizerConfig& tokenizer,
			const NormalizerConfig& normalizer,
			PositionBind positionBind)=0;

	/// \brief Declare a feature to be put into the forward index used for summarization extraction.
	/// \param[in] type type name of the feature
	/// \param[in] selectexpr an expression that decribes what elements are taken from a document for this feature (tag selection in abbreviated syntax of XPath)
	/// \param[in] tokenizer selects a tokenizer by name describing how text chunks are tokenized
	/// \param[in] normalizer selects a normalizer by name describing how tokens are normalized
	/// \param[in] positionBind influences the assingment of document position of terms produced
	virtual void addForwardIndexFeature(
			const std::string& type,
			const std::string& selectexpr,
			const TokenizerConfig& tokenizer,
			const NormalizerConfig& normalizer,
			PositionBind positionBind)=0;

	/// \brief Declare a feature to be put into the meta data table used for restrictions, weighting and summarization.
	/// \param[in] fieldname name of the field in the meta data table this feature is written to
	/// \param[in] selectexpr an expression that decribes what elements are taken from a document for this feature (tag selection in abbreviated syntax of XPath)
	/// \param[in] tokenizer selects a tokenizer by name describing how text chunks are tokenized
	/// \param[in] normalizer selects a normalizer by name describing how tokens are normalized
	/// \remark The field in the meta data table must exist before calling this function
	virtual void defineMetaData(
			const std::string& fieldname,
			const std::string& selectexpr,
			const TokenizerConfig& tokenizer,
			const NormalizerConfig& normalizer)=0;

	/// \brief Declare a feature to be defined as document attribute used for summarization (document title, document id, etc.)
	/// \param[in] attribname name of the document attribute this feature is written as.
	/// \param[in] selectexpr an expression that decribes what elements are taken from a document for this feature (tag selection in abbreviated syntax of XPath)
	/// \param[in] tokenizer selects a tokenizer by name describing how text chunks are tokenized
	/// \param[in] normalizer selects a normalizer by name describing how tokens are normalized
	/// \remark Attributes must be defined uniquely per document
	virtual void defineAttribute(
			const std::string& attribname,
			const std::string& selectexpr,
			const TokenizerConfig& tokenizer,
			const NormalizerConfig& normalizer)=0;

	/// \brief Declare a sub document for the handling of multi part documents in an analyzed content
	/// \param[in] selectexpr an expression that defines the content of the sub document declared
	/// \param[in] subDocumentTypeName type name assinged to this sub document
	/// \remark Sub documents are defined as the sections selected by the expression plus some data selected not belonging to any sub document.
	virtual void defineSubDocument(
			const std::string& subDocumentTypeName,
			const std::string& selectexpr)=0;

	/// \brief Segment and tokenize a document, assign types to tokens and metadata and normalize their values
	/// \param[in] content document content string to analyze
	/// \return the analyzed document
	/// \remark Do not use this function in case of a multipart document (defined with 'defineSubDocument(const std::string&,const std::string&)') because you get only one sub document analyzed. Use the interface created with 'createDocumentAnalyzerInstance(std::istream&)const' instead.
	virtual analyzer::Document analyze( const std::string& content) const=0;

	/// \brief Analyzer a document (same as 'DocumentAnalyzerInterface::analyze( const std::string&) const' but with stream as input)
	/// \param[in] stream with content document content string to analyze
	/// \return the analyzed document
	/// \remark Do not use this function in case of a multipart document (defined with 'defineSubDocument(const std::string&,const std::string&)') because you get only one sub document analyzed. Use the interface created with 'createDocumentAnalyzerInstance(std::istream&)const' instead.
	virtual analyzer::Document analyze( std::istream& input) const=0;

	/// \brief Create an instance of the context used for analyzing multipart documents
	/// \return the analyzer context (ownership to caller)
	virtual DocumentAnalyzerInstanceInterface* createDocumentAnalyzerInstance( std::istream& input) const=0;
};

}//namespace
#endif

