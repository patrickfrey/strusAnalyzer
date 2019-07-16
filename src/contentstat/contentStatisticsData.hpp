/*
* Copyright (c) 2018 Patrick P. Frey
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
/// \brief Structure describing the statics analysis of a document
/// \file contentStatisticsData.hpp
#ifndef _STRUS_ANALYZER_CONTENT_STATISTICS_DATA_HPP_INCLUDED
#define _STRUS_ANALYZER_CONTENT_STATISTICS_DATA_HPP_INCLUDED
#include "strus/analyzer/contentStatisticsItem.hpp"
#include "strus/contentStatisticsInterface.hpp"
#include "strus/base/thread.hpp"
#include <vector>
#include <string>
#include <cstring>
#include <map>
#include <set>

/// \brief strus toplevel namespace
namespace strus {

/// \brief Structure used to collect data for content statistics
class ContentStatisticsData
{
public:
	typedef analyzer::ContentStatisticsItem Item;

	/// \brief Default constructor
	ContentStatisticsData(){}
	/// \brief Constructor
	ContentStatisticsData( const ContentStatisticsData& o)
		:m_itemar(o.m_itemar),m_docidset(o.m_docidset),m_globalselectmap(o.m_globalselectmap),m_docselectmap(o.m_docselectmap){}
	
	/// \brief Destructor
	~ContentStatisticsData(){}

	/// \brief Set the MIME type of the document class
	/// \param[in] docid_ document identifier
	/// \param[in] select_ selection expression
	/// \param[in] type_ name of the type assigned to the item added
	/// \param[in] example_ example that led to this decision
	void addItem( const std::string& docid_, const std::string& select_, const std::string& type_, const std::string& example_)
	{
		strus::scoped_lock lock( m_mutex);
		m_docidset.insert( docid_);
		bool isnew = addMapItem( m_docselectmap, m_itemar, docid_ + "\1" + select_ + "\1" + type_, Item( select_, type_, example_, 0/*df*/, 1/*tf*/));
		addMapItem( m_globalselectmap, m_itemar, select_ + "\1" + type_, Item( select_, type_, example_, isnew?1:0/*df*/, 1/*tf*/));
	}

	/// \brief Get the global statistics
	/// \return items of global statistics
	std::vector<Item> getGlobalStatistics()
	{
		strus::scoped_lock lock( m_mutex);
		std::vector<Item> rt;
		std::map<std::string,int>::const_iterator di = m_globalselectmap.begin(), de = m_globalselectmap.end();
		for (; di != de; ++di)
		{
			rt.push_back( m_itemar[ di->second]);
		}
		return rt;
	}

	std::size_t nofDocuments() const
	{
		strus::scoped_lock lock( m_mutex);
		return m_docidset.size();
	}

private:
	typedef std::map<std::string,int> Map;

	static bool addMapItem( Map& map, std::vector<Item>& itemar, const std::string& key, const Item& item)
	{
		Map::const_iterator di = map.find( key);
		if (di == map.end())
		{
			map[ key] = itemar.size();
			itemar.push_back( item);
			return true;
		}
		else
		{
			Item& defitem = itemar[ di->second];
			defitem.incr_df( item.df());
			defitem.incr_tf( 1);
			if (defitem.example().size() < item.example().size()
			|| (defitem.example().size() == item.example().size() && defitem.example() > item.example()))
			{
				defitem.setExample( item.example());
				defitem.setType( item.type());
			}
			return false;
		}
	}
	mutable strus::mutex m_mutex;
	std::vector<Item> m_itemar;
	std::set<std::string> m_docidset;
	std::map<std::string,int> m_globalselectmap;
	std::map<std::string,int> m_docselectmap;
};

}//namespace
#endif

