/*
 * Copyright (c) 2016 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Structure desribing a result item of a token pattern matcher
/// \file "patternMatcherResultItem.hpp"
#ifndef _STRUS_ANALYZER_PATTERN_MATCHER_RESULT_ITEM_HPP_INCLUDED
#define _STRUS_ANALYZER_PATTERN_MATCHER_RESULT_ITEM_HPP_INCLUDED
#include "strus/analyzer/position.hpp"
#include "strus/base/stdint.h"

namespace strus {
namespace analyzer {

/// \brief Result item structure of a pattern match result
class PatternMatcherResultItem
{
public:
	/// \brief Default constructor
	PatternMatcherResultItem()
		:m_name(0),m_value(0),m_ordpos(0),m_ordend(0),m_origpos(),m_origend(){}
	/// \brief Constructor
	PatternMatcherResultItem( const char* name_, const char* value_, int ordpos_, int ordend_, const Position& origpos_, const Position& origend_)
		:m_name(name_),m_value(value_),m_ordpos(ordpos_),m_ordend(ordend_),m_origpos(origpos_),m_origend(origend_){}
	/// \brief Copy constructor
	PatternMatcherResultItem( const PatternMatcherResultItem& o)
		:m_name(o.m_name),m_value(o.m_value),m_ordpos(o.m_ordpos),m_ordend(o.m_ordend),m_origpos(o.m_origpos),m_origend(o.m_origend){}
	/// \brief Destructor
	~PatternMatcherResultItem(){}

	/// \brief Name of the item, defined by the variable assigned to the match
	const char* name() const		{return m_name;}
	/// \brief Pointer to value of the item
	/// \note The value is a null terminated string containing a constructed value of the result string or NULL if no such construction (format string) is defined.
	const char* value() const		{return m_value;}
	/// \brief Ordinal (counting) position of the match (resp. the first term of the match)
	int ordpos() const			{return m_ordpos;}
	/// \brief Ordinal (counting) end position of the match
	int ordend() const			{return m_ordend;}
	/// \brief Start of the result item in the original source
	const Position& origpos() const		{return m_origpos;}
	/// \brief End of the result item in the original source (first byte after the item)
	const Position& origend() const		{return m_origend;}

private:
	const char* m_name;
	const char* m_value;
	uint32_t m_ordpos;
	uint32_t m_ordend;
	Position m_origpos;
	Position m_origend;
};


}} //namespace
#endif

