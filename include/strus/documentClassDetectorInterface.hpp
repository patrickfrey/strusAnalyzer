/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Interface for detecting the document class of a content
/// \file documentClassDetectorInterface.hpp
#ifndef _STRUS_ANALYZER_DOCUMENT_CLASS_DETECTOR_INTERFACE_HPP_INCLUDED
#define _STRUS_ANALYZER_DOCUMENT_CLASS_DETECTOR_INTERFACE_HPP_INCLUDED
#include "strus/analyzer/documentClass.hpp"
#include <vector>
#include <string>

/// \brief strus toplevel namespace
namespace strus
{

/// \brief Defines a detector that returns a content description for a document content it recognizes
class DocumentClassDetectorInterface
{
public:
	/// \brief Destructor
	virtual ~DocumentClassDetectorInterface(){}

	/// \brief Scans the start of a document to detect its classification attributes (mime type, etc.)
	/// \param[in,out] dclass document class to edit
	/// \param[in] contentBegin start of content begin chunk
	/// \param[in] contentBeginSize size of content begin chunk
	/// \param[in] isComplete true, of the chunk passed is the whole document (this might influence the result)
	/// \return true, if the document class was recognized
	/// \note It is assumed that a reasonable size of the document chunk (e.g. 1K) is enough to detect the document class. This is an assumption that is wrong for many MIME types, but it should work for text content. At least it should be enough to recognize the segmenter to use.
	virtual bool detect( analyzer::DocumentClass& dclass, const char* contentBegin, std::size_t contentBeginSize, bool isComplete) const=0;
};

}//namespace
#endif

