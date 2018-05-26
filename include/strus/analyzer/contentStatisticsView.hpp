/*
 * Copyright (c) 2018 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Structures describing the internal representation of an aggregator in a document analyzer
/// \note The internal representation may not be suitable for reconstructing the object
/// \file contentStatisticsView.hpp
#ifndef _STRUS_ANALYZER_CONTENT_STATISTICS_VIEW_HPP_INCLUDED
#define _STRUS_ANALYZER_CONTENT_STATISTICS_VIEW_HPP_INCLUDED
#include "strus/analyzer/contentStatisticsElementView.hpp"

/// \brief strus toplevel namespace
namespace strus {
/// \brief analyzer parameter and return value objects namespace
namespace analyzer {

/// \brief Structure describing the internal representation of a content statistics library
/// \note The internal representation may not be suitable for reconstructing the object
class ContentStatisticsView
{
public:
	/// \brief Default constructor
	ContentStatisticsView(){}

	/// \brief Copy constructor
	ContentStatisticsView( const ContentStatisticsView& o)
		:m_elements(o.m_elements){}

	/// \brief Constructor
	/// \param[in] elements_ list of elements
	ContentStatisticsView( const std::vector<ContentStatisticsElementView>& elements_)
		:m_elements(elements_){}

	/// \brief Get the function
	const std::vector<ContentStatisticsElementView>& elements() const	{return m_elements;}

private:
	std::vector<ContentStatisticsElementView> m_elements;
};

}}//namespace
#endif

