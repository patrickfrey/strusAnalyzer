/*
 * Copyright (c) 2018 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Exported functions of the strus standard content statistics library
/// \file contentstats_std.hpp
#ifndef _STRUS_ANALYZER_CONTENT_STATISTICS_STD_LIB_HPP_INCLUDED
#define _STRUS_ANALYZER_CONTENT_STATISTICS_STD_LIB_HPP_INCLUDED

/// \brief strus toplevel namespace
namespace strus
{

/// \brief Forward declaration
class DocumentClassDetectorInterface;
/// \brief Forward declaration
class ErrorBufferInterface;
/// \brief Forward declaration
class TextProcessorInterface;
/// \brief Forward declaration
class ContentStatisticsInterface;

/// \brief Get the standard content statistics
/// \return the standard content statistics interface (with ownership)
ContentStatisticsInterface* createContentStatistics_std(
		const TextProcessorInterface* textproc,
		const DocumentClassDetectorInterface* detector,
		ErrorBufferInterface* errorhnd);

}
#endif
