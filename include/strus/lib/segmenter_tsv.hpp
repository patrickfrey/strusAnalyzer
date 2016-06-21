/*
 * Copyright (c) 2014 Patrick P. Frey
 * Copyright (c) 2016 Andreas Baumann
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Exported functions of the strus XML segmenter library based on cjson
/// \file segmenter_cjson.hpp
#ifndef _STRUS_ANALYZER_SEGMENTER_TSV_LIB_HPP_INCLUDED
#define _STRUS_ANALYZER_SEGMENTER_TSV_LIB_HPP_INCLUDED
#include <string>

/// \brief strus toplevel namespace
namespace strus {

/// \brief Forward declaration
class SegmenterInterface;
/// \brief Forward declaration
class ErrorBufferInterface;

/// \brief Get a document segmenter using tab-separated files as input
/// \return the segmenter
SegmenterInterface* createSegmenter_tsv( ErrorBufferInterface* errorhnd);

}//namespace
#endif

