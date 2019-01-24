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
#include "strus/contentIteratorInterface.hpp"
#include "strus/segmenterInterface.hpp"
#include "strus/documentClassDetectorInterface.hpp"
#include "strus/textProcessorInterface.hpp"
#include "strus/errorBufferInterface.hpp"
#include "strus/base/local_ptr.hpp"
#include "private/errorUtils.hpp"

using namespace strus;
using namespace strus::analyzer;

ContentStatistics::ContentStatistics(
		const TextProcessorInterface* textproc_,
		const DocumentClassDetectorInterface* detector_,
		ErrorBufferInterface* errorhnd_)
	:m_errorhnd(errorhnd_),m_detector(detector_),m_textproc(textproc_),m_library(errorhnd_){}

ContentStatistics::~ContentStatistics(){}

void ContentStatistics::addVisibleAttribute( const std::string& name)
{
	m_library.addVisibleAttribute( name);
}

void ContentStatistics::addLibraryElement(
	const std::string& type,
	const std::string& regex,
	int priority,
	int minLength,
	int maxLength,
	TokenizerFunctionInstanceInterface* tokenizer,
	const std::vector<NormalizerFunctionInstanceInterface*>& normalizers)
{
	m_library.addElement( type, regex, priority, minLength, maxLength, tokenizer, normalizers);
}

void ContentStatistics::addSelectorExpression( const std::string& expression)
{
	try
	{
		m_expressions.push_back( expression);
	}
	CATCH_ERROR_MAP( _TXT("error adding selector expression to statistics context: %s"), *m_errorhnd);
}

ContentStatisticsContextInterface* ContentStatistics::createContext() const
{
	try
	{
		return new ContentStatisticsContext( &m_library, m_expressions, m_textproc, m_detector, m_errorhnd);
	}
	CATCH_ERROR_MAP_RETURN( _TXT("error creating content statistics context: %s"), *m_errorhnd, NULL);
}

analyzer::ContentStatisticsView ContentStatistics::view() const
{
	try
	{
		return analyzer::ContentStatisticsView( m_library.view(), m_library.collectedAttributes(), m_expressions);
	}
	CATCH_ERROR_MAP_RETURN( _TXT("error creating content statistics context introspection: %s"), *m_errorhnd, analyzer::ContentStatisticsView());
}

ContentStatisticsContext::ContentStatisticsContext( const ContentStatisticsLibrary* library_, const std::vector<std::string>& expressions_, const TextProcessorInterface* textproc_, const DocumentClassDetectorInterface* detector_, ErrorBufferInterface* errorhnd_)
	:m_errorhnd(errorhnd_),m_textproc(textproc_),m_library(library_),m_detector(detector_),m_expressions(expressions_),m_data()
{}

ContentStatisticsContext::~ContentStatisticsContext()
{}

static bool isEmptyContent( const std::string& value)
{
	char const* vi = value.c_str();
	for (; *vi && (unsigned char)*vi <= 32; ++vi){}
	return !*vi;
}

void ContentStatisticsContext::putContent(
		const std::string& docid,
		const std::string& content,
		const analyzer::DocumentClass& doctype)
{
	try
	{
		strus::scoped_lock lock( m_mutex);

		strus::local_ptr<ContentIteratorInterface> itr;
		const SegmenterInterface* segmenter;
		if (doctype.defined())
		{
			segmenter = m_textproc->getSegmenterByMimeType( doctype.mimeType());
			if (!segmenter) throw strus::runtime_error(_TXT("can't process the MIME type '%s'"), doctype.mimeType().c_str());
			itr.reset( segmenter->createContentIterator( content.c_str(), content.size(), m_library->collectedAttributes(), m_expressions, doctype));
			if (!itr.get()) throw strus::runtime_error(_TXT("failed to create content iterator for '%s'"), doctype.mimeType().c_str());
		}
		else
		{
			analyzer::DocumentClass dclass;
			if (m_detector->detect( dclass, content.c_str(), content.size(), true))
			{
				segmenter = m_textproc->getSegmenterByMimeType( dclass.mimeType());
				if (!segmenter) throw strus::runtime_error(_TXT("can't process the MIME type '%s'"), doctype.mimeType().c_str());
				itr.reset( segmenter->createContentIterator( content.c_str(), content.size(), m_library->collectedAttributes(), m_expressions, dclass));
				if (!itr.get()) throw strus::runtime_error(_TXT("failed to create content iterator for '%s'"), dclass.mimeType().c_str());
			}
			else if (m_errorhnd->hasError())
			{
				throw strus::runtime_error(_TXT("document class detection failed: %s"), m_errorhnd->fetchError());
			}
			else
			{
				throw strus::runtime_error(_TXT("cannot determine document class"));
			}
		}
		const char* selectstr;
		std::size_t selectsize;
		const char* valuestr;
		std::size_t valuesize;
		while (itr->getNext( selectstr, selectsize, valuestr, valuesize))
		{
			std::string value( valuestr, valuesize);
			std::vector<std::string> types = m_library->matches( valuestr, valuesize);
			if (types.empty())
			{
				if (!isEmptyContent( value))
				{
					m_data.addItem( docid, std::string( selectstr, selectsize), "", value);
				}
			}
			else
			{
				std::vector<std::string>::const_iterator ti = types.begin(), te = types.end();
				for (; ti != te; ++ti)
				{
					m_data.addItem( docid, std::string( selectstr, selectsize), *ti, value);
				}
			}
		}
		if (m_errorhnd->hasError())
		{
			throw std::runtime_error( m_errorhnd->fetchError());
		}
	}
	CATCH_ERROR_MAP( _TXT("error collecting content statistics: %s"), *m_errorhnd);
}

analyzer::ContentStatisticsResult ContentStatisticsContext::statistics()
{
	try
	{
		strus::scoped_lock lock( m_mutex);
		return ContentStatisticsResult( m_data.nofDocuments(), m_data.getGlobalStatistics());
	}
	CATCH_ERROR_MAP_RETURN( _TXT("error collecting content statistics: %s"), *m_errorhnd, ContentStatisticsResult());
}

int ContentStatisticsContext::nofDocuments() const
{
	strus::scoped_lock lock( m_mutex);
	return m_data.nofDocuments();
}


