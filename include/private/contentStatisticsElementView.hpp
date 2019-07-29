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
#include "strus/structView.hpp"

/// \brief strus toplevel namespace
namespace strus {
/// \brief analyzer parameter and return value objects namespace
namespace analyzer {

/// \brief Structure describing the internal representation of a content statistics library element
/// \note The internal representation may not be suitable for reconstructing the object
class ContentStatisticsElementView
	:public StructView
{
public:
	/// \brief Constructor
	/// \brief Constructor
	/// \param[in] type_ name of the function
	/// \param[in] regex_ the segmenter selection expression
	/// \param[in] tokenizer_ view of tokenizer
	/// \param[in] normalizer_ list of views of normalizers
	ContentStatisticsElementView( const std::string& type_, const std::string& regex_, int priority_, int minLen_, int maxLen_, const StructView& tokenizer_, const StructView& normalizer_)
	{
		StructView::operator()
			("type",type_)
			("regex",regex_)
			("priority",priority_)
			("minlen",minLen_)
			("maxlen",maxLen_)
			("tokenizer",tokenizer_)
			("normalizer",normalizer_)
		;
	}
};

}}//namespace
#endif

