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
	/// \param[in] start_segpos absolute segment position where the start of the token to mark has been defined (this position is not the absolute byte position in the document, but the absolute byte position of the segment where the token has been found plus the relative byte position of the token in the segment with all encoded entities or escaping resolved. The real absolute byte position can be calculated by this markup class by mapping the offset in the segment with encoded entities to the offset in the original segnment.)
	/// \param[in] start_ofs offset of the token to mark in bytes in the segment processed
	/// \param[in] end_segpos absolute segment position where the end of the to mark has been defined (this position is not the absolute byte position in the document, but the absolute byte position of the segment where the token has been found plus the relative byte position of the token in the segment with all encoded entities or escaping resolved. The real absolute byte position can be calculated by this markup class by mapping the offset in the segment with encoded entities to the offset in the original segnment.)
	/// \param[in] start_ofs offset of the token to mark in bytes in the segment processed
	/// \param[in] markup tag structure to use for markup
	/// \param[in] level sort of priority (areas with a higher level markup are superseding the ones with lovel level. It is also used as criterion to resolve conflicts)
	virtual void putMarkup(
			const SegmenterPosition& start_segpos,
			std::size_t start_ofs,
			const SegmenterPosition& end_segpos,
			std::size_t end_ofs,
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

