/*
 * Copyright (c) 2018 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Library for adding attributes to selected tags of a document (currently only implemented for XML)
/// \file attribute_tags.hpp
#ifndef _STRUS_ANALYZER_ATTRIBUTE_TAGS_LIB_HPP_INCLUDED
#define _STRUS_ANALYZER_ATTRIBUTE_TAGS_LIB_HPP_INCLUDED
#include "strus/analyzer/documentClass.hpp"
#include "private/xpathAutomaton.hpp"
#include <string>

/// \brief strus toplevel namespace
namespace strus {

/// \brief Forward declaration
class ErrorBufferInterface;

class TagAttributeMarkupInterface
{
public:
	virtual std::string getNextMarkupValue() const=0;
};

/// \brief Analyze a content and put markups on every tag matching an expression
/// \remark This function is currently only implemented for XML
/// \param[in] content the content to process
/// \param[in] documentClass document class of the content with the encoding specified
/// \param[in] selectexpr expression selecting the tag to markup with a new attribute
/// \param[in] markup class that delivers the values for markup
/// \param[in] errhnd error buffer for reporting errors/exceptions
/// \return the tagged document
std::string markupDocumentTags( const std::string& content, const analyzer::DocumentClass& documentClass, const std::string& selectexpr, TagAttributeMarkupInterface* markup, ErrorBufferInterface* errhnd);

}//namespace
#endif

