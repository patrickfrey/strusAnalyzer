/*
 * Copyright (c) 2018 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Structures describing the internal representation of an element in a query analyzer
/// \note The internal representation may not be suitable for reconstructing the object
/// \file queryElementView.hpp
#ifndef _STRUS_ANALYZER_QUERY_ELEMENT_VIEW_HPP_INCLUDED
#define _STRUS_ANALYZER_QUERY_ELEMENT_VIEW_HPP_INCLUDED
#include "strus/structView.hpp"

/// \brief strus toplevel namespace
namespace strus {
/// \brief analyzer parameter and return value objects namespace
namespace analyzer {

/// \brief Structure describing the internal representation of a feature in the document analyzer
/// \note The internal representation may not be suitable for reconstructing the object
class QueryElementView
	:public StructView
{
public:
	/// \brief Constructor
	/// \param[in] type_ name of the function
	/// \param[in] field_ list of named parameters
	/// \param[in] tokenizer_ view of tokenizer
	/// \param[in] normalizer_ list of views of normalizers
	/// \param[in] priority_ priority of the feature
	QueryElementView( const std::string& type_, const std::string& field_, const StructView& tokenizer_, const StructView& normalizer_, int priority_)
	{
		(*this)
			("type",type_)
			("field",field_)
			("tokenizer",tokenizer_)
			("normalizer",normalizer_);
		if (priority_) (*this)("priority",priority_);
	}
};

}}//namespace
#endif

