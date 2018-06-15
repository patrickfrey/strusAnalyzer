/*
 * Copyright (c) 2016 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Interface for annotation of text in one document
/// \file "tokenMarkupContextInterface.hpp"
#ifndef _STRUS_ANALYZER_TOKEN_MARKUP_CONTEXT_INTERFACE_HPP_INCLUDED
#define _STRUS_ANALYZER_TOKEN_MARKUP_CONTEXT_INTERFACE_HPP_INCLUDED
#include "strus/segmenterContextInterface.hpp"
#include "strus/analyzer/documentClass.hpp"
#include "strus/analyzer/tokenMarkup.hpp"
#include "strus/analyzer/position.hpp"
#include <vector>
#include <string>

namespace strus
{
///\brief Forward declaration
class SegmenterInstanceInterface;

/// \brief Interface for annotation of text in one document
class TokenMarkupContextInterface
{
public:
	/// \brief Destructor
	virtual ~TokenMarkupContextInterface(){}

	/// \brief Define a marker for a span in the text
	/// \param[in] start start of the item to mark 
	/// \param[in] end the end of the item to mark
	/// \param[in] markup tag structure to use for markup
	/// \param[in] level sort of priority (areas with a higher level markup are superseding the ones with lovel level. It is also used as criterion to resolve conflicts)
	virtual void putMarkup(
			const analyzer::Position& start,
			const analyzer::Position& end,
			const analyzer::TokenMarkup& markup,
			unsigned int level)=0;

	/// \brief Get the original document content with all markups declared inserted
	/// \param[in] segmenter segmenter to use for inserting document markup tags
	/// \param[in] dclass document class of document to markup
	/// \param[in] content content string of document to markup
	/// \return the marked up document content
	virtual std::string markupDocument(
			const SegmenterInstanceInterface* segmenter,
			const analyzer::DocumentClass& dclass,
			const std::string& content) const=0;
};

} //namespace
#endif

