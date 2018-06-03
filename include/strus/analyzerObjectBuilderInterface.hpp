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
#include "strus/analyzer/segmenterOptions.hpp"
#include <string>

/// \brief strus toplevel namespace
namespace strus
{

/// \brief Forward declaration
class ContentStatisticsInterface;
/// \brief Forward declaration
class DocumentAnalyzerInstanceInterface;
/// \brief Forward declaration
class DocumentAnalyzerMapInterface;
/// \brief Forward declaration
class DocumentClassDetectorInterface;
/// \brief Forward declaration
class SegmenterInterface;
/// \brief Forward declaration
class QueryAnalyzerInstanceInterface;
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

	/// \brief Creates a document analyzer object
	/// \param[in] segmenter the document segmenter to use (ownership passed)
	/// \param[in] opts (optional) options for the creation of the segmenter instance
	/// \return the document analyzer (ownership returned)
	virtual DocumentAnalyzerInstanceInterface* createDocumentAnalyzer(
			const SegmenterInterface* segmenter,
			const analyzer::SegmenterOptions& opts=analyzer::SegmenterOptions()) const=0;

	/// \brief Creates a query analyzer object
	/// \return the query analyzer (ownership returned)
	virtual QueryAnalyzerInstanceInterface* createQueryAnalyzer() const=0;

	/// \brief Creates a document analyzer map object
	/// \return the document analyzer map (ownership returned)
	virtual DocumentAnalyzerMapInterface* createDocumentAnalyzerMap() const=0;

	/// \brief Creates a document class detector object
	/// \return the document class detector (ownership returned)
	virtual DocumentClassDetectorInterface* createDocumentClassDetector() const=0;

	/// \brief Creates an object for content statistics analyzer
	/// \return the content statistics analyzer (ownership returned)
	virtual ContentStatisticsInterface* createContentStatistics() const=0;
};

}//namespace
#endif

