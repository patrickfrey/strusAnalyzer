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
#include <string>

/// \brief strus toplevel namespace
namespace strus
{

/// \brief Forward declaration
class DocumentAnalyzerInterface;
/// \brief Forward declaration
class QueryAnalyzerInterface;
/// \brief Forward declaration
class SegmenterInterface;
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

	/// \brief Creates a document segmenter object
	/// \param[in] segmenterName name of the segmenter used (if not specified, find the first one loaded or the default one)
	/// \return the document segmenter (with ownership returned)
	virtual SegmenterInterface* createSegmenter( const std::string& segmenterName=std::string()) const=0;

	/// \brief Creates a document analyzer object
	/// \param[in] segmenterName name of the segmenter used (if not specified, find the first one loaded or the default one)
	/// \return the document analyzer (with ownership returned)
	virtual DocumentAnalyzerInterface* createDocumentAnalyzer( const std::string& segmenterName=std::string()) const=0;

	/// \brief Creates a query analyzer object
	/// \return the query analyzer (with ownership returned)
	virtual QueryAnalyzerInterface* createQueryAnalyzer() const=0;
};

}//namespace
#endif

