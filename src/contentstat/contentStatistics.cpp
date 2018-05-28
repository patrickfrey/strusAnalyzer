/*
* Copyright (c) 2018 Patrick P. Frey
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
/// \brief Structure describing the statics analysis of a document
/// \file contentStatistics.cpp
#include "contentStatistics.hpp"
#include "strus/contentStatisticsContextInterface.hpp"
#include "strus/textProcessorInterface.hpp"
#include "strus/errorBufferInterface.hpp"
#include "private/errorUtils.hpp"

using namespace strus;
using namespace strus::analyzer;

ContentStatistics::ContentStatistics(
		const TextProcessorInterface* textproc_,
		const DocumentClassDetectorInterface* detector_,
		ErrorBufferInterface* errorhnd_)
	:m_errorhnd(errorhnd_),m_detector(detector_),m_textproc(textproc_),m_library(errorhnd_){}

ContentStatistics::~ContentStatistics(){}

void ContentStatistics::addLibraryElement(
	const std::string& type,
	const std::string& regex,
	int minLength,
	int maxLength,
	TokenizerFunctionInstanceInterface* tokenizer,
	const std::vector<NormalizerFunctionInstanceInterface*>& normalizers)
{
	m_library.addElement( type, regex, minLength, maxLength, tokenizer, normalizers);
}

ContentStatisticsContextInterface* ContentStatistics::createContext() const
{
	try
	{
		return new ContentStatisticsContext( &m_library, m_textproc, m_errorhnd);
	}
	CATCH_ERROR_MAP_RETURN( _TXT("error creating content statistics context: %s"), *m_errorhnd, NULL);
}

analyzer::ContentStatisticsView ContentStatistics::view() const
{
	try
	{
		return analyzer::ContentStatisticsView( m_library.view());
	}
	CATCH_ERROR_MAP_RETURN( _TXT("error creating content statistics context introspection: %s"), *m_errorhnd, analyzer::ContentStatisticsView());
}

ContentStatisticsContext::ContentStatisticsContext( const ContentStatisticsLibrary* library_, const TextProcessorInterface* textproc_, ErrorBufferInterface* errorhnd_)
	:m_errorhnd(errorhnd_),m_textproc(textproc_),m_library(library_),m_data()
{}

ContentStatisticsContext::~ContentStatisticsContext()
{}

void ContentStatisticsContext::putContent(
		const std::string& docid,
		const std::string& content,
		const analyzer::DocumentClass& doctype)
{
	try
	{
	}
	CATCH_ERROR_MAP( _TXT("error collecting content statistics: %s"), *m_errorhnd);
}


std::vector<analyzer::ContentStatisticsItem> ContentStatisticsContext::statistics()
{
	try
	{
		return m_data.getGlobalStatistics();
	}
	CATCH_ERROR_MAP_RETURN( _TXT("error collecting content statistics: %s"), *m_errorhnd, std::vector<analyzer::ContentStatisticsItem>());
}

int ContentStatisticsContext::nofDocuments() const
{
	return m_data.nofDocuments();
}

