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
#include "strus/structView.hpp"

/// \brief strus toplevel namespace
namespace strus {
/// \brief analyzer parameter and return value objects namespace
namespace analyzer {

/// \brief Structure describing the internal representation of a document analyzer for introspection
/// \note The internal representations may not be suitable for reconstructing the objects
class QueryAnalyzerView
	:public StructView
{
public:
	/// \brief Constructor
	/// \param[in] elements_ elements
	/// \param[in] lexems_ lexems for pattern matching
	QueryAnalyzerView(
			const StructView& elements_,
			const StructView& lexems_)
		:StructView( StructView::Structure)
	{
		if (elements_.defined()) (*this)( "element", elements_);
		if (lexems_.defined()) (*this)( "lexem", lexems_);
	}
};

}}//namespace
#endif

