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
#ifndef _STRUS_ANALYZER_QUERY_ANALYZER_INTERFACE_HPP_INCLUDED
#define _STRUS_ANALYZER_QUERY_ANALYZER_INTERFACE_HPP_INCLUDED
#include "strus/analyzer/term.hpp"
#include "strus/normalizerConfig.hpp"
#include "strus/tokenizerConfig.hpp"
#include <vector>
#include <string>

namespace strus
{

/// \brief Defines a program for analyzing chunks of a query
class QueryAnalyzerInterface
{
public:
	/// \brief Destructor
	virtual ~QueryAnalyzerInterface(){}

	/// \brief Declare a method for producing a set of query feature terms out from a text chunk
	/// \param[in] method name of the method
	/// \param[in] featureType type name of the generated terms 
	/// \param[in] tokenizer selects a tokenizer by name describing how text chunks are tokenized
	/// \param[in] normalizer selects a normalizer by name describing how tokens are normalized
	virtual void defineMethod(
			const std::string& method,
			const std::string& featureType,
			const TokenizerConfig& tokenizer,
			const NormalizerConfig& normalizer)=0;

	/// \brief Analyze a single chunk of query
	/// \param[in] method selects the method defined with defineFeature(const std::string&,const std::string&,const std::string&,const std::string&) that determines how the chunk is tokenized and normalized and what types the resulting terms get.
	/// \param[in] content string of segment to analyze
	/// \note The query language determines the segmentation of the query parts.
	virtual std::vector<analyzer::Term> analyzeSegment(
			const std::string& method,
			const std::string& content) const=0;
};

}//namespace
#endif

