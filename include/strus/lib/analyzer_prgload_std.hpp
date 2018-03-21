/*
 * Copyright (c) 2018 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Exported functions for the program loader of the analyzer (load program in a domain specific language)
#ifndef _STRUS_ANALYZER_PROGRAM_LOAD_LIB_STD_HPP_INCLUDED
#define _STRUS_ANALYZER_PROGRAM_LOAD_LIB_STD_HPP_INCLUDED
#include <string>
#include <vector>

namespace strus {

/// \brief Forward declaration
class ErrorBufferInterface;
/// \brief Forward declaration
class DocumentAnalyzerInterface;
/// \brief Forward declaration
class TextProcessorInterface;

bool load_DocumentAnalyzer_program_std( DocumentAnalyzerInterface* analyzer, const TextProcessorInterface* textproc, const std::string& content, std::vector<std::string>& warnings, ErrorBufferInterface* errorhnd);

} //namespace
#endif

