/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Exported functions of the strus text processor library (container for all types of functions needed for document and query analysis)
/// \file textproc.hpp
#ifndef _STRUS_ANALYZER_TEXT_PROCESSOR_LIB_HPP_INCLUDED
#define _STRUS_ANALYZER_TEXT_PROCESSOR_LIB_HPP_INCLUDED
#include <string>

/// \brief strus toplevel namespace
namespace strus {

/// \brief Forward declaration
class TextProcessorInterface;
/// \brief Forward declaration
class ErrorBufferInterface;

/// \brief Create a text processor
/// \return the constructed text processor
TextProcessorInterface* createTextProcessor( ErrorBufferInterface* errorhnd);

}//namespace
#endif

