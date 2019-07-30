/*
 * Copyright (c) 2018 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Structures describing the internal representation of a feature in a document analyzer
/// \note The internal representation may not be suitable for reconstructing the object
/// \file featureView.hpp
#ifndef _STRUS_ANALYZER_DOCUMENT_FEATURE_VIEW_HPP_INCLUDED
#define _STRUS_ANALYZER_DOCUMENT_FEATURE_VIEW_HPP_INCLUDED
#include "strus/structView.hpp"
#include "strus/analyzer/featureOptions.hpp"

/// \brief strus toplevel namespace
namespace strus {
/// \brief analyzer parameter and return value objects namespace
namespace analyzer {

/// \brief Structure describing the internal representation of a feature in the document analyzer
/// \note The internal representation may not be suitable for reconstructing the object
class FeatureView
	:public StructView
{
public:
	/// \brief Constructor
	/// \param[in] type_ name of the function
	/// \param[in] selectexpr_ the segmenter selection expression
	/// \param[in] tokenizer_ view of tokenizer
	/// \param[in] normalizer_ list of views of normalizers
	/// \param[in] options_ set of feature options (e.g. how to build positions)
	/// \param[in] priority_ feature priority for features covering others
	FeatureView( const std::string& type_, const std::string& selectexpr_, const StructView& tokenizer_, const StructView& normalizer_, const FeatureOptions& options_, int priority_)
	{
		(*this)
			("type",type_)
			("select",selectexpr_)
			("tokenizer",tokenizer_)
			("normalizer",normalizer_);
		if (priority_) (*this)("priority",priority_);
		if (options_.opt()) (*this)("options",options_.view());
	}
};

}}//namespace
#endif

