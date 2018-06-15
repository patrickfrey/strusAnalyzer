/*
 * Copyright (c) 2016 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Structure desribing a result of a token pattern matcher
/// \file "patternMatcherResult.hpp"
#ifndef _STRUS_ANALYZER_PATTERN_MATCHER_RESULT_HPP_INCLUDED
#define _STRUS_ANALYZER_PATTERN_MATCHER_RESULT_HPP_INCLUDED
#include "strus/analyzer/patternMatcherResultItem.hpp"
#include "strus/base/stdint.h"
#include <string>
#include <vector>

namespace strus {
namespace analyzer {

/// \brief Structure desribing a result of a token pattern matcher
class PatternMatcherResult
	:public PatternMatcherResultItem
{
public:
	typedef analyzer::PatternMatcherResultItem Item;

	/// \brief Default constructor
	PatternMatcherResult()
		:PatternMatcherResultItem(),m_itemlist(){}
	/// \brief Constructor
	PatternMatcherResult( const char* name_, const char* value_, int ordpos_, int ordend_, const Position& origpos_, const Position& origend_, const std::vector<Item>& itemlist_=std::vector<Item>())
		:PatternMatcherResultItem(name_,value_,ordpos_,ordend_,origpos_,origend_),m_itemlist(itemlist_){}
	/// \brief Copy constructor
	PatternMatcherResult( const PatternMatcherResult& o)
		:PatternMatcherResultItem(o),m_itemlist(o.m_itemlist){}
	/// \brief Destructor
	~PatternMatcherResult(){}

	/// \brief List of result items defined by variables assigned to nodes of the pattern of the match
	const std::vector<Item>& items() const		{return m_itemlist;}

private:
	std::vector<Item> m_itemlist;
};


}} //namespace
#endif
