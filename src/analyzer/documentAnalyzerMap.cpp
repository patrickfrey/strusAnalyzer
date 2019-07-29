/*
 * Copyright (c) 2018 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Implementation of a parametrizable document analyzer instance able to process many different document types
/// \file documentAnalyzerMap.cpp
#include "documentAnalyzerMap.hpp"
#include "strus/documentAnalyzerContextInterface.hpp"
#include "strus/errorBufferInterface.hpp"
#include "strus/debugTraceInterface.hpp"
#include "strus/textProcessorInterface.hpp"
#include "strus/analyzerObjectBuilderInterface.hpp"
#include "private/documentAnalyzerMapView.hpp"
#include "private/internationalization.hpp"
#include "private/errorUtils.hpp"

using namespace strus;

static std::string getMimeSchemaKey( const std::string& mimeType, const std::string& schema)
{
	std::string rt;
	rt.append( mimeType);
	rt.push_back( ';');
	rt.append( schema);
	return rt;
}

static std::pair<std::string,std::string> getMimeSchemaKeyParts( const std::string& key)
{
	char const* ki = std::strchr( key.c_str(), ';');
	if (ki == NULL) throw strus::runtime_error(_TXT("currupt key in map: %s"), key.c_str());
	return std::pair<std::string,std::string>( std::string(key.c_str(),ki-key.c_str()), ki+1);
}

DocumentAnalyzerInstanceInterface* DocumentAnalyzerMap::createAnalyzer(
		const std::string& mimeType,
		const std::string& schema) const
{
	try
	{
		const TextProcessorInterface* textproc = m_objbuilder->getTextProcessor();
		if (!textproc) throw std::runtime_error( m_errorhnd->fetchError());
		const SegmenterInterface* segmenter = textproc->getSegmenterByMimeType( mimeType);
		if (!segmenter) throw std::runtime_error( m_errorhnd->fetchError());
		analyzer::SegmenterOptions segmenteropts = textproc->getSegmenterOptions( schema);
		if (m_errorhnd->hasError()) throw std::runtime_error( m_errorhnd->fetchError());
		return m_objbuilder->createDocumentAnalyzer( segmenter, segmenteropts);
	}
	CATCH_ERROR_MAP_RETURN( _TXT("error creating document analyzer instance: %s"), *m_errorhnd, NULL);
}

void DocumentAnalyzerMap::addAnalyzer(
		const std::string& mimeType_,
		const std::string& schema_,
		DocumentAnalyzerInstanceInterface* analyzer_)
{
	try
	{
		DocumentAnalyzerReference analyzer( analyzer_);
		m_analyzers.push_back( analyzer);
		if (schema_.empty())
		{
			m_mimeTypeAnalyzerMap[ mimeType_] = analyzer.get();
		}
		else
		{
			m_schemaAnalyzerMap[ getMimeSchemaKey( mimeType_, schema_)] = analyzer.get();
		}
	}
	CATCH_ERROR_MAP( _TXT("error adding analyzer to map: %s"), *m_errorhnd);
}

const DocumentAnalyzerInstanceInterface* DocumentAnalyzerMap::getAnalyzer( const std::string& mimeType, const std::string& schema) const
{
	try
	{
		Map::const_iterator ai;
		if (schema.empty())
		{
			ai = m_mimeTypeAnalyzerMap.find( mimeType);
		}
		else
		{
			ai = m_schemaAnalyzerMap.find( getMimeSchemaKey( mimeType, schema));
			if (ai == m_schemaAnalyzerMap.end())
			{
				ai = m_mimeTypeAnalyzerMap.find( mimeType);
			}
		}
		if (ai == m_mimeTypeAnalyzerMap.end())
		{
			throw strus::runtime_error(_TXT("no analyzer defined for this document class: mime-type=\"%s\", schema=\"%s\""),
							mimeType.c_str(), schema.c_str());
		}
		return ai->second;
	}
	CATCH_ERROR_MAP_RETURN( _TXT("error getting the analyzer for a document class: %s"), *m_errorhnd, NULL);
}

analyzer::Document DocumentAnalyzerMap::analyze(
			const std::string& content,
			const analyzer::DocumentClass& dclass) const
{
	try
	{
		const DocumentAnalyzerInstanceInterface* analyzer = getAnalyzer( dclass.mimeType(), dclass.schema());
		if (!analyzer) return analyzer::Document();
		return analyzer->analyze( content, dclass);
	}
	CATCH_ERROR_MAP_RETURN( _TXT("error analyzing document: %s"), *m_errorhnd, analyzer::Document());
}

DocumentAnalyzerContextInterface* DocumentAnalyzerMap::createContext(
		const analyzer::DocumentClass& dclass) const
{
	try
	{
		const DocumentAnalyzerInstanceInterface* analyzer = getAnalyzer( dclass.mimeType(), dclass.schema());
		if (!analyzer) return NULL;
		return analyzer->createContext( dclass);
	}
	CATCH_ERROR_MAP_RETURN( _TXT("error creating analyzer context: %s"), *m_errorhnd, NULL);
}

StructView DocumentAnalyzerMap::view() const
{
	try
	{
		StructView definitions;
		Map::const_iterator si = m_schemaAnalyzerMap.begin(), se = m_schemaAnalyzerMap.end();
		for (; si != se; ++si)
		{
			std::pair<std::string,std::string> kp = getMimeSchemaKeyParts( si->first);
			definitions( analyzer::DocumentAnalyzerMapElementView( kp.first, kp.second, si->second->view()));
		}
		si = m_mimeTypeAnalyzerMap.begin(), se = m_mimeTypeAnalyzerMap.end();
		for (; si != se; ++si)
		{
			definitions( analyzer::DocumentAnalyzerMapElementView( si->first, ""/*schema*/, si->second->view()));
		}
		return analyzer::DocumentAnalyzerMapView( definitions);
	}
	CATCH_ERROR_MAP_RETURN( _TXT("error in introspection: %s"), *m_errorhnd, StructView());
}



