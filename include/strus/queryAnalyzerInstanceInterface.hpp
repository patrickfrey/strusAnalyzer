/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Parameterizable query analyzer interface
/// \file queryAnalyzerInstanceInterface.hpp
#ifndef _STRUS_ANALYZER_QUERY_ANALYZER_INTERFACE_HPP_INCLUDED
#define _STRUS_ANALYZER_QUERY_ANALYZER_INTERFACE_HPP_INCLUDED
#include "strus/normalizerFunctionInstanceInterface.hpp"
#include "strus/tokenizerFunctionInstanceInterface.hpp"
#include "strus/structView.hpp"
#include <vector>
#include <string>

/// \brief strus toplevel namespace
namespace strus
{
/// \brief Forward declaration
class QueryAnalyzerContextInterface;

/// \brief Defines a program for analyzing chunks of a query
class QueryAnalyzerInstanceInterface
{
public:
	/// \brief Destructor
	virtual ~QueryAnalyzerInstanceInterface(){}

	/// \brief Declare an element to be retrieved from the search index
	/// \param[in] termtype term type name of the feature
	/// \param[in] fieldtype type of the field of this element in the query
	/// \param[in] tokenizer tokenizer (ownership passed to this) to use for this feature
	/// \param[in] normalizers list of normalizers (element ownership passed to this) to use for this feature
	/// \param[in] priority element priority analyzer element with lower priority are ousted if they are completely covered by elements with higher priority
	virtual void addElement(
			const std::string& termtype,
			const std::string& fieldtype,
			TokenizerFunctionInstanceInterface* tokenizer,
			const std::vector<NormalizerFunctionInstanceInterface*>& normalizers,
			int priority)=0;

	/// \brief Get the query term types declared in order of appearance in declarations
	/// return the query field types
	virtual std::vector<std::string> queryTermTypes() const=0;

	/// \brief Get the query field types declared in order of appearance in declarations
	/// return the query field types
	virtual std::vector<std::string> queryFieldTypes() const=0;

	/// \brief Create the context used for analyzing a query
	/// \return the query analyzer context (with ownership)
	virtual QueryAnalyzerContextInterface* createContext() const=0;

	/// \brief Return a structure with all definitions for introspection
	/// \return the structure with all definitions for introspection
	virtual StructView view() const=0;
};

}//namespace
#endif

