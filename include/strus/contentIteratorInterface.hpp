/*
 * Copyright (c) 2018 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Interface for content chunks for collecting statistics
/// \file contentIteratorInterface.hpp
#ifndef _STRUS_ANALYZER_CONTENT_ITERATOR_INTERFACE_HPP_INCLUDED
#define _STRUS_ANALYZER_CONTENT_ITERATOR_INTERFACE_HPP_INCLUDED
#include "strus/analyzer/documentClass.hpp"
#include "strus/analyzer/contentStatisticsItem.hpp"
#include <vector>
#include <string>

/// \brief strus toplevel namespace
namespace strus
{

/// \brief Defines an iterator on content provided by a segmenter
/// \note Used for content statistics analysis
class ContentIteratorInterface
{
public:
	/// \brief Destructor
	virtual ~ContentIteratorInterface(){}

	/// \brief Fetch next content item
	/// \param[in] expression selector scope expression of the chunk
	/// \param[in] segment segment content of the chunk
	/// \return true, if more, false on error or EOF
	/// \note returned chunks only valid after the call, owned by the segmenter
	virtual bool getNext(
			const char*& expression, std::size_t& expressionsize,
			const char*& segment, std::size_t& segmentsize)=0;
};

}//namespace
#endif

