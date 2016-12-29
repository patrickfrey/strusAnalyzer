/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "documentAnalyzerContext.hpp"
#include "strus/normalizerFunctionInstanceInterface.hpp"
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
	:m_segmentProcessor(analyzer_->featureConfigMap(),analyzer_->patternFeatureConfigMap())
	,m_preProcPatternMatchContextMap(analyzer_->preProcPatternMatchConfigMap())
	,m_postProcPatternMatchContextMap(analyzer_->postProcPatternMatchConfigMap())
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
		int nofSegments = 0;
	
		// [1] Scan the document and push the normalized tokenization of the elements to the result:
		while (m_segmenter->getNext( featidx, m_curr_position, segsrc, segsrcsize))
		{
			try
			{
#ifdef STRUS_LOWLEVEL_DEBUG
				std::cout << "fetch document segment '" << featidx << "': " << std::string(segsrc,segsrcsize>100?100:segsrcsize) << std::endl;
#endif
				if (featidx >= SubDocumentEnd)
				{
					//... start or end of document marker
					if (featidx == SubDocumentEnd)
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
					++nofSegments;
					if (featidx >= OfsPatternMatchSegment)
					{
						PreProcPatternMatchContext& ppctx
							= m_preProcPatternMatchContextMap.context( featidx - OfsPatternMatchSegment);
						std::size_t rel_position = (std::size_t)(m_curr_position - m_start_position);
						ppctx.process( rel_position, segsrc, segsrcsize);
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
						}
						else
						{
							std::size_t rel_position = (std::size_t)(m_curr_position - m_start_position);
							m_segmentProcessor.processDocumentSegment(
									featidx, rel_position, segsrc, segsrcsize);
						}
					}
				}
			}
			catch (const std::runtime_error& err)
			{
				std::string chunk( segsrc, segsrcsize);
				throw strus::runtime_error( _TXT( "error in analyze when processing chunk (%s): %s"), chunk.c_str(), err.what());
			}
		}
		if (!m_eof)
		{
			if (!have_document)
			{
				m_subdocstack.push_back( analyzer::Document());
				m_subdocstack.back().swap( doc);
				return false;
			}
		}
		if (nofSegments)
		{
			// process concatenated chunks:
			m_segmentProcessor.processConcatenated();

			// fetch pre processing pattern outputs:
			PreProcPatternMatchContextMap::iterator
				vi = m_preProcPatternMatchContextMap.begin(),
				ve = m_preProcPatternMatchContextMap.end();
			for (; vi != ve; ++vi)
			{
				m_segmentProcessor.processPatternMatchResult( vi->fetchResults());
			}

			// process post processing patterns:
			PostProcPatternMatchContextMap::iterator
				pi = m_postProcPatternMatchContextMap.begin(),
				pe = m_postProcPatternMatchContextMap.end();
			for (; pi != pe; ++pi)
			{
				std::vector<std::string> lexems = pi->m_feeder->lexemTypes();
				std::vector<std::string>::const_iterator li, le = lexems.end();
				for (li = lexems.begin(); li != le; ++li)
				{
					if (m_analyzer->searchIndexTermTypeSet().find(*li) == m_analyzer->searchIndexTermTypeSet().end()) break;
				}
				if (li != le) pi->process( m_segmentProcessor.searchTerms());
	
				for (li = lexems.begin(); li != le; ++li)
				{
					if (m_analyzer->forwardIndexTermTypeSet().find(*li) == m_analyzer->forwardIndexTermTypeSet().end()) break;
				}
				if (li != le) pi->process( m_segmentProcessor.forwardTerms());
	
				pi->process( m_segmentProcessor.patternLexemTerms());
				m_segmentProcessor.processPatternMatchResult( pi->fetchResults());
			}

			// create output (with real positions):
			doc = m_segmentProcessor.fetchDocument( doc);
	
			// Map statistics, if defined
			mapStatistics( doc);

			m_segmentProcessor.clearTermMaps();
		}
		else
		{
			goto AGAIN;
		}
		return true;
	}
	CATCH_ERROR_MAP_RETURN( _TXT("error in DocumentAnalyzerContext::analyzeNext: %s"), *m_errorhnd, false);
}

