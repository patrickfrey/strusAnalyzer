/*
 * Copyright (c) 2018 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Library for adding attributes to selected tags of a document (currently only implemented for XML)
/// \file markup_document_tags.hpp
#ifndef _STRUS_ANALYZER_ATTRIBUTE_TAGS_LIB_HPP_INCLUDED
#define _STRUS_ANALYZER_ATTRIBUTE_TAGS_LIB_HPP_INCLUDED
#include "strus/analyzer/documentClass.hpp"
#include "strus/analyzer/documentAttribute.hpp"
#include "strus/reference.hpp"
#include "private/xpathAutomaton.hpp"
#include <string>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <memory>

/// \brief strus toplevel namespace
namespace strus {

/// \brief Forward declaration
class ErrorBufferInterface;
/// \brief Forward declaration
class TextProcessorInterface;

class TagAttributeMarkupInterface
{
public:
	virtual strus::analyzer::DocumentAttribute synthesizeAttribute( const std::string& tagname, const std::vector<strus::analyzer::DocumentAttribute>& attributes) const=0;
};

class DocumentTagMarkupDef
{
public:
	/// \brief implementation of the markup generation
	const TagAttributeMarkupInterface* markup() const	{return m_markup.get();}
	/// \brief expression selecting the tag or sibling attribute to markup with a new attribute
	const std::string& selectexpr() const			{return m_selectexpr;}

	/// \brief Constructor
	DocumentTagMarkupDef( TagAttributeMarkupInterface* markup_, const std::string& selectexpr_)
		:m_markup(markup_),m_selectexpr(selectexpr_)
	{
		if (!m_markup.get()) throw std::bad_alloc();
	}

private:
	strus::Reference<TagAttributeMarkupInterface> m_markup;
	std::string m_selectexpr;
};


/// \brief Analyze a content and put markups on every tag matching an expression
/// \remark This function is currently only implemented for XML
/// \param[in] documentClass document class of the content with the encoding specified
/// \param[in] content the content to process
/// \param[in] markups array of definitions for markup
/// \param[in] textproc text processor interface
/// \param[in] errorhnd error buffer for reporting errors/exceptions
/// \return the tagged document
std::string markupDocumentTags( const analyzer::DocumentClass& documentClass, const std::string& content, const std::vector<DocumentTagMarkupDef>& markups, const TextProcessorInterface* textproc, ErrorBufferInterface* errorhnd);

}//namespace
#endif

