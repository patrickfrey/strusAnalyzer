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
#include "strus/analyzer/documentClass.hpp"
#include <string>
#include <vector>

namespace strus {

/// \brief Forward declaration
class ErrorBufferInterface;
/// \brief Forward declaration
class DocumentAnalyzerInstanceInterface;
/// \brief Forward declaration
class TextProcessorInterface;
/// \brief Forward declaration
class DocumentAnalyzerMapInterface;

/// \brief Load a program given as source without includes to an analyzer
/// \param[in,out] analyzer analyzer object to instrument
/// \param[in] source source with definitions
/// \param[in,out] errorhnd buffer for reporting errors (exceptions)
/// \return true on success, false on failure (inspect errorhnd for errors)
bool load_DocumentAnalyzer_program_std( DocumentAnalyzerInstanceInterface* analyzer, const TextProcessorInterface* textproc, const std::string& content, ErrorBufferInterface* errorhnd);

/// \brief Load a program given as source file name to an analyzer, recursively expanding include directives (C preprocessor style) at the beginning of the source to load
/// \param[in,out] analyzer analyzer object to instrument
/// \param[in] filename name of the file to load
/// \param[in,out] errorhnd buffer for reporting errors (exceptions)
/// \return true on success, false on failure (inspect errorhnd for errors)
bool load_DocumentAnalyzer_programfile_std( DocumentAnalyzerInstanceInterface* analyzer, const TextProcessorInterface* textproc, const std::string& filename, ErrorBufferInterface* errorhnd);

/// \brief Test if a file is an analyzer program file
/// \param[in] textproc text processor interface to determine the path of the filename
/// \param[in] filename name of the file to load
/// \param[in,out] errorhnd buffer for reporting errors (exceptions)
/// \return true on success, false on failure (inspect errorhnd for errors)
bool is_DocumentAnalyzer_programfile( const TextProcessorInterface* textproc, const std::string& filename, ErrorBufferInterface* errorhnd);

/// \brief Test if a file is an analyzer program file
/// \param[in] filename name of the file to load
/// \param[in,out] errorhnd buffer for reporting errors (exceptions)
/// \return true on success, false on failure (inspect errorhnd for errors)
bool is_DocumentAnalyzer_program( const std::string& source, ErrorBufferInterface* errorhnd);

/// \brief Load a map of definitions describing how different document types are mapped to an analyzer program from its source
/// \param[in,out] analyzermap map of analyzers to instrument
/// \param[in] textproc text processor interface to determine the path of filenames
/// \param[in] source source with definitions
/// \param[in,out] errorhnd buffer for reporting errors (exceptions)
/// \return true on success, false on failure
bool load_DocumentAnalyzerMap_program(
		DocumentAnalyzerMapInterface* analyzermap,
		const TextProcessorInterface* textproc,
		const std::string& source,
		ErrorBufferInterface* errorhnd);

/// \brief Load a map of definitions describing how different document types are mapped to an analyzer program from a file
/// \param[in,out] analyzermap map of analyzers to instrument
/// \param[in] textproc text processor interface to determine the path of filenames
/// \param[in] filename source with definitions
/// \param[in,out] errorhnd buffer for reporting errors (exceptions)
/// \return true on success, false on failure
bool load_DocumentAnalyzerMap_programfile(
		DocumentAnalyzerMapInterface* analyzermap,
		const TextProcessorInterface* textproc,
		const std::string& filename,
		ErrorBufferInterface* errorhnd);

} //namespace
#endif

