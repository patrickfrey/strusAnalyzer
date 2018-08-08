/*
 * Copyright (c) 2018 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Structure describing the internal representation of a sub content definition in the document analyzer
/// \note The internal representation may not be suitable for reconstructing the object
/// \file subContentDefinitionView.hpp
#ifndef _STRUS_ANALYZER_SUB_CONTENT_DEFINITION_VIEW_HPP_INCLUDED
#define _STRUS_ANALYZER_SUB_CONTENT_DEFINITION_VIEW_HPP_INCLUDED
#include "strus/analyzer/documentClass.hpp"

/// \brief strus toplevel namespace
namespace strus {
/// \brief analyzer parameter and return value objects namespace
namespace analyzer {

/// \brief Structure describing the internal representation of a sub content definition in the document analyzer
/// \note The internal representation may not be suitable for reconstructing the object
class SubContentDefinitionView
{
public:
	/// \brief Default constructor
	SubContentDefinitionView(){}
	/// \brief Copy constructor
	SubContentDefinitionView( const SubContentDefinitionView& o)
		:m_selectexpr(o.m_selectexpr),m_documentClass(o.m_documentClass){}

	/// \brief Constructor
	/// \param[in] selectexpr_ the segmenter selection expression
	/// \param[in] documentClass_ document class of the subcontent
	SubContentDefinitionView( const std::string& selectexpr_, const analyzer::DocumentClass& documentClass_)
		:m_selectexpr(selectexpr_),m_documentClass(documentClass_){}

	/// \brief Get the name of this sub document type
	/// \return name of the sub document type
	const analyzer::DocumentClass& documentClass() const	{return m_documentClass;}

	/// \brief Get the segmenter selection expression
	/// \return the selection expression
	const std::string& selection() const			{return m_selectexpr;}

private:
	std::string m_selectexpr;
	analyzer::DocumentClass m_documentClass;
};

}}//namespace
#endif

