/*
 * Copyright (c) 2018 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Structures describing the internal representation of a feature in a document analyzer
/// \note The internal representation may not be suitable for reconstructing the object
/// \file contentStatisticsElementView.hpp
#ifndef _STRUS_ANALYZER_CONTENT_STATISTICS_ELEMENT_VIEW_HPP_INCLUDED
#define _STRUS_ANALYZER_CONTENT_STATISTICS_ELEMENT_VIEW_HPP_INCLUDED
#include "strus/analyzer/functionView.hpp"

/// \brief strus toplevel namespace
namespace strus {
/// \brief analyzer parameter and return value objects namespace
namespace analyzer {

/// \brief Structure describing the internal representation of a content statistics library element
/// \note The internal representation may not be suitable for reconstructing the object
class ContentStatisticsElementView
{
public:
	/// \brief Default constructor
	ContentStatisticsElementView(){}
	/// \brief Copy constructor
	ContentStatisticsElementView( const ContentStatisticsElementView& o)
		:m_type(o.m_type),m_regex(o.m_regex),m_priority(o.m_priority),m_minLen(o.m_minLen),m_maxLen(o.m_maxLen),m_tokenizer(o.m_tokenizer),m_normalizer(o.m_normalizer){}

	/// \brief Constructor
	/// \brief Constructor
	/// \param[in] type_ name of the function
	/// \param[in] regex_ the segmenter selection expression
	/// \param[in] tokenizer_ view of tokenizer
	/// \param[in] normalizer_ list of views of normalizers
	ContentStatisticsElementView( const std::string& type_, const std::string& regex_, int priority_, int minLen_, int maxLen_, const FunctionView& tokenizer_, const std::vector<FunctionView>& normalizer_)
		:m_type(type_),m_regex(regex_),m_priority(priority_),m_minLen(minLen_),m_maxLen(maxLen_),m_tokenizer(tokenizer_),m_normalizer(normalizer_){}

	/// \brief Get the type
	const std::string& type() const				{return m_type;}
	/// \brief Get the segmenter selection expression
	const std::string& regex() const			{return m_regex;}
	/// \brief Get the priority
	int priority() const					{return m_priority;}
	/// \brief Get the priority
	int minLen() const					{return m_minLen;}
	/// \brief Get the priority
	int maxLen() const					{return m_maxLen;}
	/// \brief Get the tokenizer
	const FunctionView& tokenizer() const			{return m_tokenizer;}
	/// \brief Get the list of normalizers
	const std::vector<FunctionView>& normalizer() const	{return m_normalizer;}

private:
	std::string m_type;
	std::string m_regex;
	int m_priority;
	int m_minLen;
	int m_maxLen;
	FunctionView m_tokenizer;
	std::vector<FunctionView> m_normalizer;
};

}}//namespace
#endif

