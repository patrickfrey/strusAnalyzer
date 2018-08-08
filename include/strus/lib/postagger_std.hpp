/*
 * Copyright (c) 2018 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Library for tagging documents with POS( part of speech) tagging info fed from a 3rdParty source (e.g. standford POS tagger, google syntaxnet, etc.)
/// \file analyzer_postagger_std.hpp
#ifndef _STRUS_ANALYZER_LIB_POS_TAGGER_STD_HPP_INCLUDED
#define _STRUS_ANALYZER_LIB_POS_TAGGER_STD_HPP_INCLUDED
#include <string>
#include <vector>

/// \brief strus toplevel namespace
namespace strus
{

/// \brief Forward declaration
class TokenizerFunctionInstanceInterface;
/// \brief Forward declaration
class ErrorBufferInterface;
/// \brief Forward declaration
class PosTaggerDataInterface;
/// \brief Forward declaration
class PosTaggerInterface;


/// \brief Create an interface for building up the data to tag documents with
/// \param[in] tokenizer tokenizer interface to use (passed with ownership)
/// \param[in] errorhnd error buffer interface for exceptions thrown
/// \return the structure to collect POS tagging output
PosTaggerDataInterface* createPosTaggerData_standard( TokenizerFunctionInstanceInterface* tokenizer, ErrorBufferInterface* errorhnd);

/// \brief Create an interface for the construction of a POS tagger instance for a specified segmenter
/// \param[in] errorhnd error buffer interface for exceptions thrown
/// \return the POS tagger base interface
PosTaggerInterface* createPosTagger_standard( ErrorBufferInterface* errorhnd);

}//namespace
#endif

