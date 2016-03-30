/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Interface for the execution context of a document analyzer
/// \file documentAnalyzerContextInterface.hpp
#ifndef _STRUS_ANALYZER_DOCUMENT_ANALYZER_CONTEXT_INTERFACE_HPP_INCLUDED
#define _STRUS_ANALYZER_DOCUMENT_ANALYZER_CONTEXT_INTERFACE_HPP_INCLUDED
#include "strus/analyzer/document.hpp"
#include <vector>
#include <string>

/// \brief strus toplevel namespace
namespace strus
{

/// \brief Defines the context for analyzing multi part documents, iterating on the sub documents defined, splitting them into normalized terms that can be fed to the strus IR engine
class DocumentAnalyzerContextInterface
{
public:
	/// \brief Destructor
	virtual ~DocumentAnalyzerContextInterface(){}

	/// \brief Feed the analyzer with the next chunk of input to process
	/// \param[in] chunk pointer to input chunk to process
	/// \param[in] chunksize size of input chunk to process in bytes
	/// \param[in] eof true, if this chunk fed is the last one in input
	virtual void putInput( const char* chunk, std::size_t chunksize, bool eof)=0;

	/// \brief Analyze the next sub document from the input feeded with putInput(const char*,std::size_t)
	/// \param[out] doc the analyzed sub document structure
	/// \return true, if the next document could be fetched, false if more input has to be fed or no input left (EOF)
	virtual bool analyzeNext( analyzer::Document& doc)=0;
};

}//namespace
#endif
