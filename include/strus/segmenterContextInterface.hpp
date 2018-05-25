/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Interface for the execution context of a document segmenter
/// \file segmenterContextInterface.hpp
#ifndef _STRUS_ANALYZER_SEGMENTER_CONTEXT_INTERFACE_HPP_INCLUDED
#define _STRUS_ANALYZER_SEGMENTER_CONTEXT_INTERFACE_HPP_INCLUDED
#include <utility>
#include <string>

#ifdef _MSC_VER
#include <BaseTsd.h>
namespace strus {
	///\typedef SegmenterPosition
	///\brief Byte position in scanned source
	typedef INT64 SegmenterPosition;
}//namespace
#else
#include <stdint.h>
namespace strus {
	///\typedef SegmenterPosition
	///\brief Byte position in scanned source
	typedef int64_t SegmenterPosition;
}//namespace
#endif

/// \brief strus toplevel namespace
namespace strus
{

/// \class SegmenterContextInterface
/// \brief Defines the context for segmenting one document
class SegmenterContextInterface
{
public:
	/// \brief Destructor
	virtual ~SegmenterContextInterface(){}

	/// \brief Feed the segmenter with the next chunk of input to process
	/// \param[in] chunk pointer to input chunk to process (to copy by this)
	/// \param[in] chunksize size of input chunk to process in bytes
	/// \param[in] eof true, if this is the last chunk to feed
	/// \remark the buffer passed to this must be copied by the segmenter, because it is not guaranteed to survive till the next call of putInput.
	virtual void putInput( const char* chunk, std::size_t chunksize, bool eof)=0;

	/// \brief Fetch the next text segment
	/// \param[out] id identifier of the expression that addresses the text segment (defined with SegmenterInterface::defineSelectorExpression(int, const std::string&) or with SegmenterInterface::defineSubSection(int,int,const std::string&))
	/// \param[out] pos position of the segment in the original source
	/// \param[out] segment pointer to the start of the segment.
	/// \param[out] segmentsize size of segment in bytes
	/// \return true, if a valid segment could be returned, false in case of no segments left or more required to be fed
	/// \remark the segments must be delivered in ascending order of positions. Segments with same position can be returned in any order
	virtual bool getNext( int& id, SegmenterPosition& pos, const char*& segment, std::size_t& segmentsize)=0;
};

}//namespace
#endif

