/*
 * Copyright (c) 2018 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Structure describing the internal representation of a sub document definition in the document analyzer
/// \note The internal representation may not be suitable for reconstructing the object
/// \file subDocumentDefinitionView.hpp
#ifndef _STRUS_ANALYZER_SUB_DOCUMENT_DEFINITION_VIEW_HPP_INCLUDED
#define _STRUS_ANALYZER_SUB_DOCUMENT_DEFINITION_VIEW_HPP_INCLUDED
#include "strus/structView.hpp"

/// \brief strus toplevel namespace
namespace strus {
/// \brief analyzer parameter and return value objects namespace
namespace analyzer {

/// \brief Structure describing the internal representation of a sub document definition in the document analyzer
/// \note The internal representation may not be suitable for reconstructing the object
class SubDocumentDefinitionView
	:public StructView
{
public:
	/// \brief Constructor
	/// \param[in] name_ name of the sub document type
	/// \param[in] selectexpr_ the segmenter selection expression
	SubDocumentDefinitionView( const std::string& name_, const std::string& selectexpr_)
	{
		(*this)("name",name_)("select",selectexpr_);
	}
};

}}//namespace
#endif

