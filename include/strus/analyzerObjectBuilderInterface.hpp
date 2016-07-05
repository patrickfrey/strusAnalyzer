/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Builder object for all toplevel interfaces related to the analyzer. Used by components acting as proxy (calling strus with RPC) or by components that build the analyzer universe from external components (loading objects from dynamically loadable modules)
/// \file analyzerObjectBuilderInterface.hpp
#ifndef _STRUS_ANALYZER_OBJECT_BUILDER_INTERFACE_HPP_INCLUDED
#define _STRUS_ANALYZER_OBJECT_BUILDER_INTERFACE_HPP_INCLUDED
#include "strus/segmenterOptions.hpp"
#include <string>

/// \brief strus toplevel namespace
namespace strus
{

/// \brief Forward declaration
class DocumentAnalyzerInterface;
/// \brief Forward declaration
class SegmenterInterface;
/// \brief Forward declaration
class QueryAnalyzerInterface;
/// \brief Forward declaration
class TextProcessorInterface;

/// \brief Interface providing a mechanism to create complex multi component objects for the document and query analysis in strus.
class AnalyzerObjectBuilderInterface
{
public:
	/// \brief Destructor
	virtual ~AnalyzerObjectBuilderInterface(){}

	/// \brief Get the analyzer text processor interface
	/// \return the analyzer text processor interface reference
	virtual const TextProcessorInterface* getTextProcessor() const=0;

	/// \brief Get a loaded document segmenter object reference
	/// \param[in] segmenterName name of the segmenter used (if empty, find the first one loaded or the default one)
	/// \return a read only document segmenter reference
	virtual const SegmenterInterface* getSegmenter( const std::string& segmenterName) const=0;

	/// \brief Get a loaded document segmenter object reference that is able to process the specified MIME type
	/// \param[in] mimetype MIME type of the document type to process
	/// \return a read only document segmenter reference
	virtual const SegmenterInterface* findMimeTypeSegmenter( const std::string& mimetype) const=0;

	/// \brief Creates a document analyzer object
	/// \param[in] segmenter the document segmenter to use (ownership passed)
	/// \param[in] opts (optional) options for the creation of the segmenter instance
	/// \return the document analyzer (ownership returned)
	virtual DocumentAnalyzerInterface* createDocumentAnalyzer(
			const SegmenterInterface* segmenter,
			const SegmenterOptions& opts=SegmenterOptions()) const=0;

	/// \brief Creates a query analyzer object
	/// \return the query analyzer (ownership returned)
	virtual QueryAnalyzerInterface* createQueryAnalyzer() const=0;
};

}//namespace
#endif

