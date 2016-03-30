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
#include <string>

/// \brief strus toplevel namespace
namespace strus {

/// \brief Forward declaration
class DocumentAnalyzerInterface;
/// \brief Forward declaration
class QueryAnalyzerInterface;
/// \brief Forward declaration
class SegmenterInterface;
/// \brief Forward declaration
class AnalyzerErrorBufferInterface;

/// \brief Creates a parameterizable analyzer instance for analyzing documents
/// \param[in] segmenter segmenter type to be used by the created analyzer.
/// \return the analyzer program (with ownership)
DocumentAnalyzerInterface* createDocumentAnalyzer( const SegmenterInterface* segmenter, AnalyzerErrorBufferInterface* errorhnd);

/// \brief Creates a parameterizable analyzer instance for analyzing queries
/// \return the analyzer program (with ownership)
QueryAnalyzerInterface* createQueryAnalyzer( AnalyzerErrorBufferInterface* errorhnd);

}//namespace
#endif

