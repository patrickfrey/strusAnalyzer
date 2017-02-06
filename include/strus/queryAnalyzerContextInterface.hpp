/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Interface for the execution context of a query analyzer
/// \file queryAnalyzerContextInterface.hpp
#ifndef _STRUS_ANALYZER_QUERY_ANALYZER_CONTEXT_INTERFACE_HPP_INCLUDED
#define _STRUS_ANALYZER_QUERY_ANALYZER_CONTEXT_INTERFACE_HPP_INCLUDED
#include "strus/analyzer/query.hpp"
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
		GroupEvery,		///< Every element gets its own group
		GroupAll		///< All elements are grouped together
	};

	/// \brief Group elements of the query together
	/// \param[in] groupid identifier of the grouping operation defined by the caller that gets into the resulting query as operator identifier of the query instructions
	/// \param[in] fieldnolist number of the fields to take as arguments
	/// \param[in] groupBy how to group elements together
	/// \param[in] groupSingle true, if this operator should also be applied on single elements, false, if the operator is not applied on single argument elements and the elements apear as they are
	/// \note This method influences how a query is iterated on
	virtual void groupElements( unsigned int groupid, const std::vector<unsigned int>& fieldnoList, const GroupBy& groupBy, bool groupSingle)=0;

	/// \brief Analyze the query feeded with putField(unsigned int,const std::string&,const std::string&) and groupElements(const std::string&,const std::vector<unsigned int>&,const GroupBy&)
	/// \return the query structure
	virtual analyzer::Query analyze()=0;
};

}//namespace
#endif

