/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Interface for the execution context of a document analyzer
/// \file documentAnalyzerContextInterface.hpp
#ifndef _STRUS_ANALYZER_QUERY_ANALYZER_CONTEXT_INTERFACE_HPP_INCLUDED
#define _STRUS_ANALYZER_QUERY_ANALYZER_CONTEXT_INTERFACE_HPP_INCLUDED
#include "strus/analyzer/document.hpp"
#include <vector>
#include <string>

/// \brief strus toplevel namespace
namespace strus
{

/// \brief Defines the context for analyzing queries for the strus IR engine
class QueryAnalyzerContextInterface
{
public:
	/// \brief Destructor
	virtual ~QueryAnalyzerContextInterface(){}

	/// \brief Feed the analyzer with the a field of the query
	/// \param[in] fieldno number of the field
	/// \param[in] fieldtype type of the field
	/// \param[in] content content of the field
	virtual void putField( unsigned int fieldno, const std::string& fieldtype, const std::string& content)=0;

	/// \brief Descriptor that tells how to group elements together
	enum GroupBy
	{
		GroupByPosition,	///< Elements with same position are grouped together
		GroupAll		///< All elements are grouped together
	};

	/// \brief Group elements of the query together
	/// \param[in] name of the group operator
	/// \param[in] fieldnolist number of the fields to take as arguments
	/// \param[in] groupBy how to group elements together
	/// \note This method influences how a query is iterated on
	virtual void groupElements( const std::string& name, const std::vector<unsigned int>& fieldnoList, const GroupBy& groupBy)=0;

	/// \brief Analyze the query feeded with putInput(const char*,std::size_t)
	/// \return the query structure
	virtual analyzer::Query analyze()=0;
};

}//namespace
#endif

