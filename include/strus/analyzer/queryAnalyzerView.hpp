/*
 * Copyright (c) 2018 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Structure describing the internal representation of a document analyzer for introspection
/// \note The internal representations may not be suitable for reconstructing the objects
/// \file queryAnalyzerView.hpp
#ifndef _STRUS_ANALYZER_QUERY_ANALYZER_VIEW_HPP_INCLUDED
#define _STRUS_ANALYZER_QUERY_ANALYZER_VIEW_HPP_INCLUDED
#include "strus/analyzer/queryElementView.hpp"

/// \brief strus toplevel namespace
namespace strus {
/// \brief analyzer parameter and return value objects namespace
namespace analyzer {

/// \brief Structure describing the internal representation of a document analyzer for introspection
/// \note The internal representations may not be suitable for reconstructing the objects
class QueryAnalyzerView
{
public:
	/// \brief Default constructor
	QueryAnalyzerView(){}
	/// \brief Copy constructor
	QueryAnalyzerView( const QueryAnalyzerView& o)
		:m_elements(o.m_elements)
		,m_patternLexems(o.m_patternLexems)
		{}
	/// \brief Constructor
	/// \param[in] elements_ elements
	QueryAnalyzerView(
			const std::vector<QueryElementView>& elements_,
			const std::vector<QueryElementView>& patternLexems_)
		:m_elements(elements_)
		,m_patternLexems(patternLexems_)
		{}

	const std::vector<QueryElementView>& elements() const		{return m_elements;}
	const std::vector<QueryElementView>& patternLexems() const	{return m_patternLexems;}

private:
	std::vector<QueryElementView> m_elements;
	std::vector<QueryElementView> m_patternLexems;
};

}}//namespace
#endif

