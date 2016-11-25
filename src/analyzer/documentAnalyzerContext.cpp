/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "documentAnalyzerContext.hpp"
#include "strus/normalizerFunctionContextInterface.hpp"
#include "strus/normalizerFunctionInstanceInterface.hpp"
#include "strus/tokenizerFunctionContextInterface.hpp"
#include "strus/tokenizerFunctionInstanceInterface.hpp"
#include "strus/segmenterContextInterface.hpp"
#include "strus/segmenterInstanceInterface.hpp"
#include "strus/errorBufferInterface.hpp"
#include "private/utils.hpp"
#include "private/errorUtils.hpp"
#include "private/internationalization.hpp"
#include <stdexcept>
#include <sstream>
#include <iostream>
#include <set>
#include <map>
#include <cstring>
#include <limits>

#undef STRUS_LOWLEVEL_DEBUG

using namespace strus;

DocumentAnalyzerContext::DocumentAnalyzerContext( const DocumentAnalyzer* analyzer_, const analyzer::DocumentClass& dclass, ErrorBufferInterface* errorhnd)
	:m_segmentProcessor(analyzer_->featureConfigMap())
	,m_analyzer(analyzer_)
	,m_segmenter(m_analyzer->segmenter()->createContext( dclass))
	,m_eof(false)
	,m_curr_position(0)
	,m_start_position(0)
	,m_errorhnd(errorhnd)
{
	if (!m_segmenter)
	{
		throw strus::runtime_error( _TXT("failed to create document analyzer context"));
	}
	m_subdocstack.push_back( analyzer::Document());
}

DocumentAnalyzerContext::~DocumentAnalyzerContext()
{
	delete m_segmenter;
}

void DocumentAnalyzerContext::putInput( const char* chunk, std::size_t chunksize, bool eof)
{
	m_segmenter->putInput( chunk, chunksize, eof);
	m_eof = eof;
}

void DocumentAnalyzerContext::mapStatistics( analyzer::Document& res) const
{
	std::vector<DocumentAnalyzer::StatisticsConfig>::const_iterator
		si = m_analyzer->statisticsConfigs().begin(), se = m_analyzer->statisticsConfigs().end();
	for (; si != se; ++si)
	{
		NumericVariant value = si->statfunc()->evaluate( res);
#ifdef STRUS_LOWLEVEL_DEBUG
		std::cout << "add aggregated metadata " << si->name() << " " << value.tostring().c_str() << std::endl;
#endif
		res.setMetaData( si->name(), value);
	}
}

bool DocumentAnalyzerContext::analyzeNext( analyzer::Document& doc)
{
	try 
	{
	AGAIN:
		if (m_subdocstack.empty())
		{
			return false;
		}
		bool have_document = false;
		doc.clear();
		m_subdocstack.back().swap( doc);
		m_subdocstack.pop_back();
		const char* segsrc = 0;
		std::size_t segsrcsize = 0;
		int featidx = 0;
	
		// [1] Scan the document and push the normalized tokenization of the elements to the result:
		while (m_segmenter->getNext( featidx, m_curr_position, segsrc, segsrcsize))
		{
			try
			{
#ifdef STRUS_LOWLEVEL_DEBUG
				std::cout << "fetch document segment '" << featidx << "': " << std::string(segsrc,segsrcsize>100?100:segsrcsize) << std::endl;
#endif
				if (featidx >= EndOfSubDocument)
				{
					//... start or end of document marker
					if (featidx == EndOfSubDocument)
					{
						//... end of sub document -> out of loop and return document
						have_document = true;
						break;
					}
					else
					{
						// process what is left to process for the current sub document:
						m_segmentProcessor.processConcatenated();
						doc = m_segmentProcessor.fetchDocument( doc);
						m_segmentProcessor.clearTermMaps();
	
						// create new sub document:
						m_subdocstack.push_back( doc);
						doc.setSubDocumentTypeName( m_analyzer->subdoctypes()[ featidx-OfsSubDocument]);
						m_start_position = m_curr_position;
					}
				}
				else
				{
					const FeatureConfig& feat = m_analyzer->featureConfigMap().featureConfig( featidx);
					if (feat.tokenizer()->concatBeforeTokenize())
					{
						// concat chunks that need to be concatenated before tokenization:
						std::size_t rel_position = (std::size_t)(m_curr_position - m_start_position);
						m_segmentProcessor.concatDocumentSegment(
								featidx, rel_position, segsrc, segsrcsize);
						continue;
					}
					else
					{
						std::size_t rel_position = (std::size_t)(m_curr_position - m_start_position);
						m_segmentProcessor.processDocumentSegment(
								featidx, rel_position, segsrc, segsrcsize);
					}
				}
			}
			catch (const std::runtime_error& err)
			{
				std::string chunk( segsrc, segsrcsize);
				throw strus::runtime_error( _TXT( "error in analyze when processing chunk (%s): %s"), chunk.c_str(), err.what());
			}
		}
		if (!m_eof && !have_document)
		{
			m_subdocstack.push_back( analyzer::Document());
			m_subdocstack.back().swap( doc);
			return false;
		}
	
		// process concatenated chunks:
		m_segmentProcessor.processConcatenated();
		// create output (with real positions):
		doc = m_segmentProcessor.fetchDocument( doc);

		// Map statistics, if defined
		bool rt = (doc.metadata().size() + doc.attributes().size() + doc.searchIndexTerms().size() + doc.forwardIndexTerms().size() != 0);
		if (rt)
		{
			mapStatistics( doc);
		}
		m_segmentProcessor.clearTermMaps();
		if (have_document && !rt)
		{
#ifdef STRUS_LOWLEVEL_DEBUG
			std::cout << "got empty document" << std::endl;
#endif
			goto AGAIN;
		}
		return rt;
	}
	CATCH_ERROR_MAP_RETURN( _TXT("error in DocumentAnalyzerContext::analyzeNext: %s"), *m_errorhnd, false);
}
