/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Exported functions of the strus analyzer library
/// \file analyzer.hpp
#ifndef _STRUS_ANALYZER_LIB_HPP_INCLUDED
#define _STRUS_ANALYZER_LIB_HPP_INCLUDED
#include "strus/analyzer/segmenterOptions.hpp"
#include <string>

/// \brief strus toplevel namespace
namespace strus {

/// \brief Forward declaration
class DocumentAnalyzerInstanceInterface;
/// \brief Forward declaration
class DocumentAnalyzerMapInterface;
/// \brief Forward declaration
class QueryAnalyzerInstanceInterface;
/// \brief Forward declaration
class SegmenterInterface;
/// \brief Forward declaration
class TextProcessorInterface;
/// \brief Forward declaration
class ErrorBufferInterface;
/// \brief Forward declaration
class AnalyzerObjectBuilderInterface;

/// \brief Creates a parameterizable analyzer instance for analyzing documents
/// \param[in] segmenter segmenter type to be used by the created analyzer.
/// \param[in] textproc text processor for creating functions and resources needed for analysis
/// \param[in] segmenter segmenter type
/// \param[in] opts options for the segmenter
/// \param[in] errorhnd error buffer interface
/// \return the analyzer program (with ownership)
DocumentAnalyzerInstanceInterface* createDocumentAnalyzer( const TextProcessorInterface* textproc, const SegmenterInterface* segmenter, const analyzer::SegmenterOptions& opts, ErrorBufferInterface* errorhnd);

/// \brief Creates a parameterizable analyzer instance for analyzing queries
/// \param[in] errorhnd error buffer interface
/// \return the analyzer program (with ownership)
QueryAnalyzerInstanceInterface* createQueryAnalyzer( ErrorBufferInterface* errorhnd);

/// \brief Creates a analyzer map for bundling different instances of analyzers for different classes of documents
/// \param[in] objbuilder analyzer object builder interface
/// \param[in] errorhnd error buffer interface
/// \return the analyzer program (with ownership)
DocumentAnalyzerMapInterface* createDocumentAnalyzerMap( const AnalyzerObjectBuilderInterface* objbuilder, ErrorBufferInterface* errorhnd);

}//namespace
#endif

