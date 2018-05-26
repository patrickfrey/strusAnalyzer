/*
* Copyright (c) 2018 Patrick P. Frey
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
/// \brief Standard implementation of the content statistics interface
/// \file contentStatistics.hpp
#ifndef _STRUS_ANALYZER_CONTENT_STATISTICS_IMPLEMENTATION_HPP_INCLUDED
#define _STRUS_ANALYZER_CONTENT_STATISTICS_IMPLEMENTATION_HPP_INCLUDED
#include "strus/contentStatisticsInterface.hpp"
#include "strus/contentStatisticsContextInterface.hpp"
#include "contentStatisticsLibrary.hpp"
#include "contentStatisticsData.hpp"

/// \brief strus toplevel namespace
namespace strus {

/// \brief Forward declaration
class ErrorBufferInterface;
/// \brief Forward declaration
class TextProcessorInterface;
/// \brief Forward declaration
class DocumentClassDetectorInterface;

/// \brief Implementation of content statistics
class ContentStatistics
	:public ContentStatisticsInterface
{
public:
	/// \brief Constructor
	ContentStatistics(
			const TextProcessorInterface* textproc_,
			const DocumentClassDetectorInterface* detector_,
			ErrorBufferInterface* errorhnd_);
	/// \brief Destructor
	virtual ~ContentStatistics();

	virtual void addLibraryElement(
			const std::string& type,
			const std::string& regex,
			int minLength,
			int maxLength,
			TokenizerFunctionInstanceInterface* tokenizer,
			const std::vector<NormalizerFunctionInstanceInterface*>& normalizers);

	virtual ContentStatisticsContextInterface* createContext() const;

	virtual analyzer::ContentStatisticsView view() const;

private:
	ErrorBufferInterface* m_errorhnd;
	const DocumentClassDetectorInterface* m_detector;
	const TextProcessorInterface* m_textproc;
	ContentStatisticsLibrary m_library;
};

/// \brief Implementation of content statistics
class ContentStatisticsContext
	:public ContentStatisticsContextInterface
{
public:
	/// \brief Constructor
	ContentStatisticsContext( const ContentStatisticsLibrary* library_, const TextProcessorInterface* textproc_, ErrorBufferInterface* errorhnd_);
	/// \brief Destructor
	virtual ~ContentStatisticsContext();

	virtual void putContent(
			const std::string& docid,
			const std::string& content,
			const analyzer::DocumentClass& doctype);

	/// \brief Do return the overall statistics of the sample documents
	virtual std::vector<analyzer::ContentStatisticsItem> statistics();

	/// \brief Get the number of sample documents
	virtual int nofDocuments() const;

private:
	ErrorBufferInterface* m_errorhnd;
	const TextProcessorInterface* m_textproc;
	const ContentStatisticsLibrary* m_library;
	ContentStatisticsData m_data;
};

}//namespace
#endif

