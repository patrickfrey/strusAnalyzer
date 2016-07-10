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
	/// \return true, on success, false on eof or error
	/// \note this method allows to iterate on content segments for implementing markups covering multiple content segments
	virtual bool getNext( SegmenterPosition& segpos, const char*& segment, std::size_t& segmentsize)=0;

	/// \brief Get the size in bytes of a segment converted to UTF-8
	/// \param[in] segpos segment position
	/// \return the size of the segment as UTF-8 in bytes
	virtual unsigned int segmentSize( const SegmenterPosition& segpos)=0;

	/// \brief Get the (tag) name of a segment specified by its position in the original source
	/// \param[in] segpos segment position
	/// \return the (tag) name of the segment
	virtual std::string tagName( const SegmenterPosition& segpos) const=0;

	/// \brief Get the (tag) hierarchy level of a segment specified by its position in the original source
	/// \param[in] segpos segment position
	/// \return the (tag) hierarchy level of the segment
	virtual int tagLevel( const SegmenterPosition& segpos) const=0;

	/// \brief Define an open tag markup in the text
	/// \param[in] segpos segment position returned by the method getNext of the segmenter context created by the same instance as this and fed with the same content
	/// \param[in] ofs byte position offset of the parsed content (UTF-8) in the segment where we want to insert the markup into
	/// \param[in] name tag name to put as markup into the content (as UTF-8)
	virtual void putOpenTag(
			const SegmenterPosition& segpos,
			std::size_t ofs,
			const std::string& name)=0;

	/// \brief Define an attribute markup in the text
	/// \param[in] segpos segment position
	/// \param[in] ofs offset ot the attribute in the text
	/// \param[in] name name of the attribute to insert (as UTF-8)
	/// \param[in] value of the attribute to insert (as UTF-8)
	virtual void putAttribute(
			const SegmenterPosition& segpos,
			std::size_t ofs,
			const std::string& name,
			const std::string& value)=0;

	/// \brief Define a close tag markup in the text
	/// \param[in] segpos segment position returned by the method getNext of the segmenter context created by the same instance as this and fed with the same content
	/// \param[in] ofs byte position offset of the parsed content (UTF-8) in the segment where we want to insert the markup into
	/// \param[in] name tag name closed to put as markup into the content (as UTF-8)
	virtual void putCloseTag(
			const SegmenterPosition& segpos,
			std::size_t ofs,
			const std::string& name)=0;

	/// \brief Get the original document content with all markups declared inserted
	/// \return the marked up document content
	virtual std::string getContent() const=0;
};

}//namespace
#endif

