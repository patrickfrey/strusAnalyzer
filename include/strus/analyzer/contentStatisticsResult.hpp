/*
* Copyright (c) 2018 Patrick P. Frey
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
/// \brief Structure describing the structure statistics analysis of a sample document collection
/// \file contentStatisticsResult.hpp
#ifndef _STRUS_ANALYZER_CONTENT_STATISTICS_RESULT_HPP_INCLUDED
#define _STRUS_ANALYZER_CONTENT_STATISTICS_RESULT_HPP_INCLUDED
#include "strus/analyzer/contentStatisticsItem.hpp"
#include <vector>

/// \brief strus toplevel namespace
namespace strus {
namespace analyzer {

/// \brief Defines the content statistics result from a collection sample
class ContentStatisticsResult
{
public:
	ContentStatisticsResult()
		:m_nofDocuments(0),m_items(){}
	ContentStatisticsResult( int nofDocuments_, const std::vector<analyzer::ContentStatisticsItem>& items_)
		:m_nofDocuments(nofDocuments_),m_items(items_){}
	ContentStatisticsResult( const ContentStatisticsResult& o)
		:m_nofDocuments(o.m_nofDocuments),m_items(o.m_items){}

	///\brief Get the number of sample documents
	int nofDocuments() const						{return m_nofDocuments;}
	///\brief Get the statistic items
	const std::vector<analyzer::ContentStatisticsItem>& items() const	{return m_items;}

private:
	int m_nofDocuments;
	std::vector<analyzer::ContentStatisticsItem> m_items;
};

}}//namespace
#endif

