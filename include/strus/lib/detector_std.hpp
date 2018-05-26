/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Exported functions of the strus standard content detector library
/// \file detector_std.hpp
#ifndef _STRUS_ANALYZER_CONTENT_DETECTOR_STD_LIB_HPP_INCLUDED
#define _STRUS_ANALYZER_CONTENT_DETECTOR_STD_LIB_HPP_INCLUDED

/// \brief strus toplevel namespace
namespace strus
{

/// \brief Forward declaration
class DocumentClassDetectorInterface;
/// \brief Forward declaration
class ErrorBufferInterface;
/// \brief Forward declaration
class TextProcessorInterface;

/// \brief Get the standard content detector (with ownership)
/// \return the content detector class
DocumentClassDetectorInterface* createDetector_std( TextProcessorInterface* textproc, ErrorBufferInterface* errorhnd);

}
#endif

