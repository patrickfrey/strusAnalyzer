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
/// \brief Parameterizable query analyzer interface
/// \file queryAnalyzerInterface.hpp
#ifndef _STRUS_ANALYZER_QUERY_ANALYZER_INTERFACE_HPP_INCLUDED
#define _STRUS_ANALYZER_QUERY_ANALYZER_INTERFACE_HPP_INCLUDED
#include "strus/analyzer/term.hpp"
#include "strus/normalizerFunctionInstanceInterface.hpp"
#include "strus/tokenizerFunctionInstanceInterface.hpp"
#include <vector>
#include <string>

/// \brief strus toplevel namespace
namespace strus
{

/// \brief Defines a program for analyzing chunks of a query
class QueryAnalyzerInterface
{
public:
	/// \brief Destructor
	virtual ~QueryAnalyzerInterface(){}

	/// \brief Declare how a set of query features is produced out from a phrase of a certain type
	/// \param[in] phraseType label of the phrase type
	/// \param[in] featureType type name (in the storage) of the generated features
	/// \param[in] tokenizer tokenizer (ownership passed to this) to use for this feature
	/// \param[in] normalizers list of normalizers (ownership of elements passed to this) to use for this feature
	/// \note It is recommended to name the phraseType as the featureType to avoid to many different namings. The phrase type is used to address the method, as the expressions in the segmenter of the document do. But in most cases there will be only one of a kind, so it does not make sense to have disinct names for it.
	virtual void definePhraseType(
			const std::string& phraseType,
			const std::string& featureType,
			TokenizerFunctionInstanceInterface* tokenizer,
			const std::vector<NormalizerFunctionInstanceInterface*>& normalizers)=0;

	/// \brief Analyze a single phrase of query
	/// \param[in] phraseType selects the feature configuration that determines how the phrase is tokenized and normalized, what types the resulting terms get and what set (as referred to in the query evaluation) the created query features are assigned to.
	/// \param[in] content string of the phrase to analyze
	/// \note The query language determines the segmentation of the query parts.
	virtual std::vector<analyzer::Term> analyzePhrase(
			const std::string& phraseType,
			const std::string& content) const=0;
};

}//namespace
#endif

