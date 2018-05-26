/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Interface for context for collecting content statistics
/// \file contentStatisticsContextInterface.hpp
#ifndef _STRUS_ANALYZER_CONTENT_STATISTICS_CONTEXT_INTERFACE_HPP_INCLUDED
#define _STRUS_ANALYZER_CONTENT_STATISTICS_CONTEXT_INTERFACE_HPP_INCLUDED
#include "strus/analyzer/documentClass.hpp"
#include "strus/analyzer/contentStatisticsItem.hpp"
#include <vector>
#include <string>

/// \brief strus toplevel namespace
namespace strus
{

/// \brief Defines a program for analyzing a document, splitting it into normalized terms that can be fed to the strus IR engine
class ContentStatisticsContextInterface
{
public:
	/// \brief Destructor
	virtual ~ContentStatisticsContextInterface(){}

	/// \brief Put content to collect statistics
	/// \param[in] docid document identifier as appearing in the result
	/// \param[in] content document content
	/// \param[in] doctype document type if defined, otherwise guessed
	virtual void putContent(
			const std::string& docid,
			const std::string& content,
			const analyzer::DocumentClass& doctype)=0;

	/// \brief Do return the overall statistics of the sample documents
	virtual std::vector<analyzer::ContentStatisticsItem> statistics()=0;

	/// \brief Get the number of sample documents
	virtual int nofDocuments() const=0;
};

}//namespace
#endif

