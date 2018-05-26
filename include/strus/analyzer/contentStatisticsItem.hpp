/*
* Copyright (c) 2018 Patrick P. Frey
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
/// \brief Structure describing an item of the structure statistics analysis of a document
/// \file contentStatisticsItem.hpp
#ifndef _STRUS_ANALYZER_CONTENT_STATISTICS_ITEM_HPP_INCLUDED
#define _STRUS_ANALYZER_CONTENT_STATISTICS_ITEM_HPP_INCLUDED
#include <string>

/// \brief strus toplevel namespace
namespace strus {
namespace analyzer {

/// \brief Defines an item describing the statistics in a collection
class ContentStatisticsItem
{
public:
	ContentStatisticsItem( const std::string& select_, const std::string& type_, const std::string& example_, int df_, int tf_)
		:m_select(select_),m_type(type_),m_example(example_),m_df(df_),m_tf(tf_){}
	ContentStatisticsItem( const ContentStatisticsItem& o)
		:m_select(o.m_select),m_type(o.m_type),m_example(o.m_example),m_df(o.m_df),m_tf(o.m_tf){}

	///\brief Get the select expression
	const std::string& select() const	{return m_select;}
	///\brief Get the type assigned to it
	const std::string& type() const		{return m_type;}
	///\brief Get an example that led to this item
	const std::string& example() const	{return m_example;}
	///\brief Get the number of documents in the collection this item appears in
	int df() const				{return m_df;}
	///\brief Get the number of appearances in the collection this item appears in
	int tf() const				{return m_tf;}

	void incr_df( int df_)			{m_df+=df_;}
	void incr_tf( int tf_)			{m_tf+=tf_;}

private:
	std::string m_select;
	std::string m_type;
	std::string m_example;
	int m_df;
	int m_tf;
};

}}//namespace
#endif

