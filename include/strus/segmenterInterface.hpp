/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Interface for the document segmenter
/// \file segmenterInterface.hpp
#ifndef _STRUS_ANALYZER_SEGMENTER_INTERFACE_HPP_INCLUDED
#define _STRUS_ANALYZER_SEGMENTER_INTERFACE_HPP_INCLUDED
#include "strus/analyzer/segmenterOptions.hpp"
#include "strus/analyzer/documentClass.hpp"
#include "strus/structView.hpp"
#include <vector>
#include <string>
#include <utility>

/// \brief strus toplevel namespace
namespace strus
{

/// \brief Forward declaration
class SegmenterInstanceInterface;
/// \brief Forward declaration
class ContentIteratorInterface;

/// \class SegmenterInterface
/// \brief Defines an interface for creating instances of programs for document segmentation
class SegmenterInterface
{
public:
	/// \brief Destructor
	virtual ~SegmenterInterface(){}

	/// \brief Get the mime type accepted by this segmenter
	/// \return the mime type string
	virtual const char* mimeType() const=0;

	/// \brief Create a parameterizable segmenter instance
	virtual SegmenterInstanceInterface* createInstance(
			const analyzer::SegmenterOptions& opts=analyzer::SegmenterOptions()) const=0;

	/// \brief Create an iterator on content for statistics
	/// \param[in] content pointer to content
	/// \param[in] contentsize size of content in bytes
	/// \param[in] attributes attributes that should be included in the path expressions of the result
	/// \param[in] expressions predefined selector expressions that are used for the outputs on a match
	/// \param[in] dclass document class (encoding)
	/// \param[in] opts segmenter options
	/// \return content iterator interface (with ownership)
	virtual ContentIteratorInterface* createContentIterator(
			const char* content,
			std::size_t contentsize,
			const std::vector<std::string>& attributes,
			const std::vector<std::string>& expressions,
			const analyzer::DocumentClass& dclass,
			const analyzer::SegmenterOptions &opts=analyzer::SegmenterOptions()) const=0;

	/// \brief Get the name of the function
	/// \return the identifier
	virtual const char* name() const=0;

	/// \brief Return a structure with all definitions for introspection
	/// \return the structure with all definitions for introspection
	virtual StructView view() const=0;
};

}//namespace
#endif


