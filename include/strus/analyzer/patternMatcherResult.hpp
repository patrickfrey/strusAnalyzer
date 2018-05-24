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
{
public:
	typedef analyzer::PatternMatcherResultItem Item;

	PatternMatcherResult()
		:m_name(0),m_value(0),m_start_ordpos(0),m_end_ordpos(0),m_start_origseg(0),m_end_origseg(0),m_start_origpos(0),m_end_origpos(0),m_itemlist(){}
	/// \brief Constructor
	PatternMatcherResult( const char* name_, const char* value_, uint32_t start_ordpos_, uint32_t end_ordpos_, uint32_t start_origseg_, uint32_t start_origpos_, uint32_t end_origseg_, uint32_t end_origpos_, const std::vector<Item>& itemlist_=std::vector<Item>())
		:m_name(name_),m_value(value_),m_start_ordpos(start_ordpos_),m_end_ordpos(end_ordpos_),m_start_origseg(start_origseg_),m_end_origseg(end_origseg_),m_start_origpos(start_origpos_),m_end_origpos(end_origpos_),m_itemlist(itemlist_){}
	/// \brief Copy constructor
	PatternMatcherResult( const PatternMatcherResult& o)
		:m_name(o.m_name),m_value(o.m_value),m_start_ordpos(o.m_start_ordpos),m_end_ordpos(o.m_end_ordpos),m_start_origseg(o.m_start_origseg),m_end_origseg(o.m_end_origseg),m_start_origpos(o.m_start_origpos),m_end_origpos(o.m_end_origpos),m_itemlist(o.m_itemlist){}
	/// \brief Destructor
	~PatternMatcherResult(){}

	/// \brief Name of the result, defined by the name of the pattern of the match
	const char* name() const			{return m_name;}
	/// \brief Pointer to value of the result
	/// \note The value is a null terminated string containing a constructed value of the result string or NULL if not such construction (format string) is defined.
	const char* value() const			{return m_value;}
	/// \brief Ordinal (counting) position of the match (resp. the first term of the match)
	uint32_t start_ordpos() const			{return m_start_ordpos;}
	/// \brief Ordinal (counting) end position of the match
	uint32_t end_ordpos() const			{return m_end_ordpos;}
	/// \brief Original segment index of the start of the result in the source
	std::size_t start_origseg() const		{return m_start_origseg;}
	/// \brief Original byte position start of the result in the source segment as UTF-8 specified with start_origseg
	std::size_t start_origpos() const		{return m_start_origpos;}
	/// \brief Original segment index of the end of the result in the source
	std::size_t end_origseg() const			{return m_end_origseg;}
	/// \brief Original byte position end of the result in the source segment as UTF-8 specified with start_origseg
	std::size_t end_origpos() const			{return m_end_origpos;}
	/// \brief List of result items defined by variables assigned to nodes of the pattern of the match
	const std::vector<Item>& items() const		{return m_itemlist;}

private:
	const char* m_name;
	const char* m_value;
	uint32_t m_start_ordpos;
	uint32_t m_end_ordpos;
	uint32_t m_start_origseg;
	uint32_t m_end_origseg;
	uint32_t m_start_origpos;
	uint32_t m_end_origpos;
	std::vector<Item> m_itemlist;
};


}} //namespace
#endif
