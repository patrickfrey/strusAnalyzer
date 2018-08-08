/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Interface for a parametrizable document analyzer instance able to process many different document types
/// \file documentAnalyzerMapInterface.hpp
#ifndef _STRUS_ANALYZER_DOCUMENT_ANALYZER_MAP_INTERFACE_HPP_INCLUDED
#define _STRUS_ANALYZER_DOCUMENT_ANALYZER_MAP_INTERFACE_HPP_INCLUDED
#include "strus/analyzer/document.hpp"
#include "strus/analyzer/documentClass.hpp"
#include "strus/analyzer/documentAnalyzerMapView.hpp"
#include <vector>
#include <string>

/// \brief strus toplevel namespace
namespace strus
{

/// \brief Forward declaration
class DocumentAnalyzerContextInterface;
/// \brief Forward declaration
class DocumentAnalyzerInstanceInterface;

/// \brief Defines a program for analyzing a document, splitting it into normalized terms that can be fed to the strus IR engine
class DocumentAnalyzerMapInterface
{
public:
	/// \brief Destructor
	virtual ~DocumentAnalyzerMapInterface(){}

	/// \brief Declare a an analyzer interface to instrument and and add with addAnalyzer
	/// \param[in] mimetype of the document for this analyzer, determines the document segmenter
	/// \param[in] scheme scheme of the document to determine the segmenter options (can be empty meaning not defined)
	/// \return the analyzer (with ownership)
	virtual DocumentAnalyzerInstanceInterface* createAnalyzer(
			const std::string& mimeType,
			const std::string& scheme) const=0;

	/// \brief Declare a an analyzer to be used for the analysis of a specific document class
	/// \param[in] mimetype of the document to process with this analyzer (must be defined)
	/// \param[in] scheme scheme of the document to process with this analyzer (can be empty meaning not defined)
	/// \param[in] analyzer analyzer to use for the defined class of documents (with ownership)
	virtual void addAnalyzer(
			const std::string& mimeType,
			const std::string& scheme,
			DocumentAnalyzerInstanceInterface* analyzer)=0;

	/// \brief Get the analyzer interface assigned to a document class
	/// \param[in] dclass description of the content type and encoding to process
	/// \return a reference to the analyzer interface
	virtual const DocumentAnalyzerInstanceInterface* getAnalyzer(
			const std::string& mimeType,
			const std::string& scheme) const=0;

	/// \brief Segment and tokenize a document, assign types to tokens and metadata and normalize their values
	/// \param[in] content document content string to analyze
	/// \param[in] dclass description of the content type and encoding to process
	/// \return the analyzed document
	/// \remark Do not use this function in case of a multipart document (defined with 'defineSubDocument(const std::string&,const std::string&)') because you get only one sub document analyzed. 
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
	virtual analyzer::DocumentAnalyzerMapView view() const=0;
};

}//namespace
#endif

