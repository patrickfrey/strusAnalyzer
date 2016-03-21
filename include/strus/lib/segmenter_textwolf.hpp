/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Exported functions of the strus XML segmenter library based on textwolf
/// \file segmenter_textwolf.hpp
#ifndef _STRUS_ANALYZER_SEGMENTER_TEXTWOLF_LIB_HPP_INCLUDED
#define _STRUS_ANALYZER_SEGMENTER_TEXTWOLF_LIB_HPP_INCLUDED
#include <string>

/// \brief strus toplevel namespace
namespace strus {

/// \brief Forward declaration
class SegmenterInterface;
/// \brief Forward declaration
class AnalyzerErrorBufferInterface;

/// \brief Get a document XML segmenter based on textwolf
/// \return the segmenter
SegmenterInterface* createSegmenter_textwolf( AnalyzerErrorBufferInterface* errorhnd);

}//namespace
#endif

