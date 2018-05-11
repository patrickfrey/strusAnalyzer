/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Interface for a parameterizable instance of a document segmenter
/// \file segmenterInstanceInterface.hpp
#ifndef _STRUS_ANALYZER_SEGMENTER_INSTANCE_INTERFACE_HPP_INCLUDED
#define _STRUS_ANALYZER_SEGMENTER_INSTANCE_INTERFACE_HPP_INCLUDED
#include "strus/analyzer/documentClass.hpp"
#include "strus/analyzer/functionView.hpp"
#include <string>

/// \brief strus toplevel namespace
namespace strus
{
/// \brief Forward declaration
class SegmenterContextInterface;
/// \brief Forward declaration
class SegmenterMarkupContextInterface;

/// \brief Defines a program for splitting a source text it into chunks with an id correspoding to a selecting expression.
class SegmenterInstanceInterface
{
public:
	/// \brief Destructor
	virtual ~SegmenterInstanceInterface(){}

	/// \brief Defines an expression for selecting chunks from a document
	/// \param[in] id identifier of the chunks that match to expression
	/// \param[in] expression expression for selecting chunks
	virtual void defineSelectorExpression( int id, const std::string& expression)=0;

	/// \brief Defines an expression for identifying a sub section of the document.
	/// \param[in] startId identifier to be returned when a sub section of this type starts
	/// \param[in] endId identifier to be returned when a sub section of this type ends
	/// \param[in] expression expression for selecting the sub section
	virtual void defineSubSection( int startId, int endId, const std::string& expression)=0;

	/// \brief Creates a context for segmenting one document of a specified class
	/// \param[in] dclass description of the document type and encoding to process
	/// \return the segmenter context object (with ownership, to be desposed with delete by the caller)
	virtual SegmenterContextInterface* createContext( const analyzer::DocumentClass& dclass) const=0;

	/// \brief Creates an instance of the segmenters document markup context
	/// \param[in] dclass description of the document type and encoding to process
	/// \param[in] content document content to process (no chunkwise processing)
	/// \return the segmenter markup object (with ownership, to be desposed with delete by the caller)
	virtual SegmenterMarkupContextInterface* createMarkupContext(
			const analyzer::DocumentClass& dclass,
			const std::string& content) const=0;

	/// \brief Get the definition of the function as structure for introspection
	/// \return structure for introspection
	virtual analyzer::FunctionView view() const=0;
};

}//namespace
#endif

