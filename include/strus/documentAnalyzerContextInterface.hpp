/*
---------------------------------------------------------------------
    The C++ library strus implements basic operations to build
    a search engine for structured search on unstructured data.

    Copyright (C) 2015 Patrick Frey

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License as published by the Free Software Foundation; either
    version 3 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

--------------------------------------------------------------------

	The latest version of strus can be found at 'http://github.com/patrickfrey/strus'
	For documentation see 'http://patrickfrey.github.com/strus'

--------------------------------------------------------------------
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
