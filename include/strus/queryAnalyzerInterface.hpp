/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
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
/// \brief Forward declaration
class QueryAnalyzerContextInterface;

/// \brief Defines a program for analyzing chunks of a query
class QueryAnalyzerInterface
{
public:
	/// \brief Destructor
	virtual ~QueryAnalyzerInterface(){}

	/// \brief Declare an element to be retrieved from the search index
	/// \param[in] termtype term type name of the feature
	/// \param[in] fieldtype type of the field of this element in the query
	/// \param[in] tokenizer tokenizer (ownership passed to this) to use for this feature
	/// \param[in] normalizers list of normalizers (element ownership passed to this) to use for this feature
	virtual void addSearchIndexElement(
			const std::string& termtype,
			const std::string& fieldtype,
			TokenizerFunctionInstanceInterface* tokenizer,
			const std::vector<NormalizerFunctionInstanceInterface*>& normalizers)=0;

	/// \brief Declare a feature to be put into the forward index used for summarization extraction.
	/// \param[in] termtype term type name of the feature
	/// \param[in] fieldtype type of the field of this element in the query
	/// \param[in] tokenizer tokenizer (ownership passed to this) to use for this feature
	/// \param[in] normalizers list of normalizers (ownership of elements passed to this) to use for this feature
	virtual void addForwardIndexElement(
			const std::string& termtype,
			const std::string& fieldtype,
			TokenizerFunctionInstanceInterface* tokenizer,
			const std::vector<NormalizerFunctionInstanceInterface*>& normalizers)=0;

	/// \brief Declare a feature to be put into the meta data table used for restrictions, weighting and summarization.
	/// \param[in] metaname name of the column in the meta data table this feature is searched in
	/// \param[in] fieldtype type of the field of this element in the query
	/// \param[in] tokenizer tokenizer (ownership passed to this) to use for this feature
	/// \param[in] normalizers list of normalizers (ownership of elements passed to this) to use for this feature
	/// \remark The field in the meta data table must exist before this function is called
	virtual void addMetaDataElement(
			const std::string& metaname,
			const std::string& fieldtype,
			TokenizerFunctionInstanceInterface* tokenizer,
			const std::vector<NormalizerFunctionInstanceInterface*>& normalizers)=0;

	/// \brief Create the context used for analyzing a query
	/// \return the query analyzer context (with ownership)
	virtual QueryAnalyzerContextInterface* createContext() const=0;
};

}//namespace
#endif

