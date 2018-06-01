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
class DocumentAnalyzerInterface;
/// \brief Forward declaration
class TextProcessorInterface;

/// \brief Description of one element of an analyzer map
struct AnalyzerMapElement
{
	AnalyzerMapElement(){}
	AnalyzerMapElement( const strus::analyzer::DocumentClass& doctype_, const std::string& segmenter_, const std::string& program_)
		:doctype(doctype_),segmenter(segmenter_),program(program_){}
	AnalyzerMapElement( const AnalyzerMapElement& o)
		:doctype(o.doctype),segmenter(o.segmenter),program(o.program){}

	strus::analyzer::DocumentClass doctype;		///< document class that identifies the segmenter
	std::string segmenter;				///< segmenter explicitely defined
	std::string program;				///< analyzer program file
};

bool load_DocumentAnalyzer_program_std( DocumentAnalyzerInterface* analyzer, const TextProcessorInterface* textproc, const std::string& content, ErrorBufferInterface* errorhnd);
bool load_DocumentAnalyzer_programfile_std( DocumentAnalyzerInterface* analyzer, const TextProcessorInterface* textproc, const std::string& filename, ErrorBufferInterface* errorhnd);

bool is_DocumentAnalyzer_programfile( const TextProcessorInterface* textproc, const std::string& filename, ErrorBufferInterface* errorhnd);

/// \brief Load a map of definitions describing how different document types are mapped to an analyzer program
/// \param[in] mapdef list of definitions to instrument
/// \param[in] source source with definitions
/// \param[in,out] errorhnd buffer for reporting errors (exceptions)
/// \return true on success, false on failure
bool load_DocumentAnalyzerMap(
		std::vector<AnalyzerMapElement>& mapdef,
		const std::string& source,
		ErrorBufferInterface* errorhnd);

} //namespace
#endif

