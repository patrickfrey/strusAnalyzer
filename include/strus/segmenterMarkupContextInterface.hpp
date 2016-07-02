/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Interface for the execution context for inserting markups into a document
/// \file segmenterMarkupContextInterface.hpp
#ifndef _STRUS_ANALYZER_SEGMENTER_MARKUP_CONTEXT_INTERFACE_HPP_INCLUDED
#define _STRUS_ANALYZER_SEGMENTER_MARKUP_CONTEXT_INTERFACE_HPP_INCLUDED
#include "strus/segmenterContextInterface.hpp"
#include <string>

/// \brief strus toplevel namespace
namespace strus
{

/// \class SegmenterMarkupContextInterface
/// \brief Defines the context for inserting markups into one document
class SegmenterMarkupContextInterface
{
public:
	/// \brief Destructor
	virtual ~SegmenterMarkupContextInterface(){}

	/// \brief Get the next content segment
	/// \param[out] segpos segment position returned 
	/// \param[out] segment pointer to start of segment
	/// \param[out] segmentsize size of segment in bytes
	/// \return the tag level of the content segment returned
	/// \note this method allows to iterate on content segments for implementing markups covering multiple content segments
	virtual int getNext( SegmenterPosition& segpos, const char*& segment, std::size_t& segmentsize)=0;

	/// \brief Define a marker in the text
	/// \param[in] segpos segment position returned by the method getNext of the segmenter context created by the same instance as this and fed with the same content
	/// \param[in] pos byte position in the segment where we want to insert the markup into
	/// \param[in] marker string to put as markup into the content (as UTF-8)
	/// \remark The segment must be of type content where markup is allowed
	virtual void putMarkup(
			std::size_t segpos,
			std::size_t pos,
			const std::string& marker)=0;

	/// \brief Get the original document content with all markups declared inserted
	/// \return the marked up document content
	virtual std::string getContent() const=0;
};

}//namespace
#endif

