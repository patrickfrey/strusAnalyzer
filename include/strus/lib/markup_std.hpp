/*
 * Copyright (c) 2016 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Exported functions of the standard document markup library
/// \file error.hpp
#ifndef _STRUS_ANALYZER_MARKUP_STD_LIB_HPP_INCLUDED
#define _STRUS_ANALYZER_MARKUP_STD_LIB_HPP_INCLUDED
#include <cstdio>

/// \brief strus toplevel namespace
namespace strus {

/// \brief Forward declaration
class TokenMarkupInstanceInterface;
/// \brief Forward declaration
class ErrorBufferInterface;

/// \brief Create the interface for markup of tokens in a document text
TokenMarkupInstanceInterface* createTokenMarkupInstance_standard(
		ErrorBufferInterface* errorhnd);

}//namespace
#endif

