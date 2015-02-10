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
/// \brief Exported functions of the strus analyzer library
#ifndef _STRUS_ANALYZER_LIB_HPP_INCLUDED
#define _STRUS_ANALYZER_LIB_HPP_INCLUDED
#include <string>

namespace strus {

/// \brief Forward declaration
class DocumentAnalyzerInterface;
/// \brief Forward declaration
class QueryAnalyzerInterface;
/// \brief Forward declaration
class TextProcessorInterface;
/// \brief Forward declaration
class SegmenterInterface;

/// \brief Creates an analyzer program for analyzing a document
/// \param[in] textProcessorInterface static const reference to the token miner. It defines the set of token type definitions that can be used in the program.
/// \param[in] segmenter empty segmenter instance to be used by the created analyzer (ownership transferred).
/// \return the analyzer program reference to be disposed with delete by the caller
DocumentAnalyzerInterface*
	createDocumentAnalyzer(
		const TextProcessorInterface* textProcessor,
		SegmenterInterface* segmenter);

/// \brief Creates an analyzer program for analyzing a query
/// \param[in] textProcessor static const reference to a token miner. It defines the set of token type definitions that can be used in the program.
/// \return the analyzer program reference to be disposed with delete by the caller
QueryAnalyzerInterface*
	createQueryAnalyzer(
		const TextProcessorInterface* textProcessor);

}//namespace
#endif

