/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "documentAnalyzerContext.hpp"
#include "strus/debugTraceInterface.hpp"
#include "strus/normalizerFunctionInstanceInterface.hpp"
#include "strus/tokenizerFunctionInstanceInterface.hpp"
#include "strus/segmenterContextInterface.hpp"
#include "strus/segmenterInstanceInterface.hpp"
#include "strus/errorBufferInterface.hpp"
#include "private/errorUtils.hpp"
#include "private/internationalization.hpp"
#include <stdexcept>
#include <sstream>
#include <iostream>
#include <set>
#include <map>
#include <cstring>
#include <limits>

#define STRUS_DBGTRACE_COMPONENT_NAME "analyzer"
#define DEBUG_OPEN( NAME) if (m_debugtrace) m_debugtrace->open( NAME);
#define DEBUG_CLOSE() if (m_debugtrace) m_debugtrace->close();
#define DEBUG_EVENT1( NAME, FMT, ID)				if (m_debugtrace) m_debugtrace->event( NAME, FMT, ID);
#define DEBUG_EVENT2( NAME, FMT, ID, VAL)			if (m_debugtrace) m_debugtrace->event( NAME, FMT, ID, VAL);
#define DEBUG_EVENT2_CONTENT( NAME, FMT, ID, VAL, STR, LEN)	if (m_debugtrace) {std::string cs(contentCut(STR,LEN,100)); m_debugtrace->event( NAME, FMT, ID, VAL, cs.c_str());}

#define B10000000 128
#define B11000000 (127+64)

static std::string contentCut( const char* str, std::size_t size, std::size_t len)
{
	if (len >= size) len = size;
	while (len && (str[ len-1] & B11000000) == B10000000) --len;
	std::string rt;
	for (std::size_t si=0; si<len; ++si)
	{
		if ((unsigned char)str[si] < 32) rt.push_back('_'); else rt.push_back(str[si]);
	}
	return rt;
}

using namespace strus;

DocumentAnalyzerContext::DocumentAnalyzerContext( const DocumentAnalyzer* analyzer_, const analyzer::DocumentClass& dclass, ErrorBufferInterface* errorhnd)
	:m_segmentProcessor(analyzer_->featureConfigMap(),analyzer_->patternFeatureConfigMap())
	,m_preProcPatternMatchContextMap(analyzer_->preProcPatternMatchConfigMap())
	,m_postProcPatternMatchContextMap(analyzer_->postProcPatternMatchConfigMap())
	,m_analyzer(analyzer_)
	,m_segmenter(m_analyzer->segmenter()->createContext( dclass))
	,m_segmenterstack()
	,m_eof(false)
	,m_curr_position_ofs(0)
	,m_curr_position(0)
	,m_start_position(0)
	,m_nof_segments(0)
	,m_subdocTypeName()
	,m_errorhnd(errorhnd)
	,m_debugtrace(0)
{
	if (!m_segmenter)
	{
		throw strus::runtime_error( "%s", _TXT("failed to create document analyzer context"));
	}
	DebugTraceInterface* dbgi = m_errorhnd->debugTrace();
	if (dbgi) m_debugtrace = dbgi->createTraceContext( STRUS_DBGTRACE_COMPONENT_NAME);
}

DocumentAnalyzerContext::~DocumentAnalyzerContext()
{
	delete m_segmenter;
	std::vector<SegmenterStackElement>::const_iterator si = m_segmenterstack.begin(), se = m_segmenterstack.begin();
	for (; si != se; ++si)
	{
		delete si->segmenter;
	}
}

void DocumentAnalyzerContext::putInput( const char* chunk, std::size_t chunksize, bool eof)
{
	m_segmenter->putInput( chunk, chunksize, eof);
	m_eof = eof;
}

void DocumentAnalyzerContext::processAggregatedMetadata( analyzer::Document& res) const
{
	std::vector<DocumentAnalyzer::StatisticsConfig>::const_iterator
		si = m_analyzer->statisticsConfigs().begin(), se = m_analyzer->statisticsConfigs().end();
	DEBUG_OPEN( "metadata");
	for (; si != se; ++si)
	{
		NumericVariant value = si->statfunc()->evaluate( res);
		res.setMetaData( si->name(), value);
		DEBUG_EVENT2( "aggregated", "%s %s", si->name().c_str(), value.tostring().c_str());
	}
	DEBUG_CLOSE();
}


void DocumentAnalyzerContext::completeDocumentProcessing( analyzer::Document& res)
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
			if (m_analyzer->searchIndexTermTypeSet().find(*li) != m_analyzer->searchIndexTermTypeSet().end()) break;
		}
		if (li != le) pi->process( m_segmentProcessor.searchTerms());

		for (li = lexems.begin(); li != le; ++li)
		{
			if (m_analyzer->forwardIndexTermTypeSet().find(*li) != m_analyzer->forwardIndexTermTypeSet().end()) break;
		}
		if (li != le) pi->process( m_segmentProcessor.forwardTerms());

		pi->process( m_segmentProcessor.patternLexemTerms());
		m_segmentProcessor.processPatternMatchResult( pi->fetchResults());
	}

	// create output (with real positions):
	res = m_segmentProcessor.fetchDocument();
	res.setSubDocumentTypeName( m_subdocTypeName);

	// Map aggregated metadata, if defined:
	processAggregatedMetadata( res);

	// Reset current document processing state:
	m_segmentProcessor.clearTermMaps();
	pi = m_postProcPatternMatchContextMap.begin();
	for (; pi != pe; ++pi)
	{
		pi->clear();
	}
	vi = m_preProcPatternMatchContextMap.begin();
	for (; vi != ve; ++vi)
	{
		vi->clear();
	}
}

bool DocumentAnalyzerContext::analyzeNext( analyzer::Document& doc)
{
	try 
	{
		doc.clear();
		const char* segsrc = 0;
		std::size_t segsrcsize = 0;
		int featidx = 0;
	
		DEBUG_OPEN( "analyze");
		for (;;)
		{
			// [1] Scan the document and push the normalized tokenization of the elements to the result:
			while (m_segmenter->getNext( featidx, m_curr_position, segsrc, segsrcsize))
			{
				m_curr_position += m_curr_position_ofs;
				try
				{
					DEBUG_EVENT2_CONTENT( "segment", "%d %d %s", (int)m_curr_position, (featidx), segsrc, segsrcsize);
					if (featidx >= SubDocumentEnd)
					{
						if (featidx >= OfsSubContent)
						{
							const DocumentAnalyzer::SubSegmenterDef* subsegmenterdef = m_analyzer->subsegmenter( featidx - OfsSubContent);
							if (subsegmenterdef)
							{
								DEBUG_EVENT2( "subcontent", "%s; charset=%s", subsegmenterdef->documentClass.mimeType().c_str(), subsegmenterdef->documentClass.encoding().c_str());
								SegmenterContextInterface* ns = subsegmenterdef->segmenterInstance->createContext( subsegmenterdef->documentClass);
								if (!ns) throw strus::runtime_error( "%s", _TXT("failed to create sub segmenter context"));
								m_segmenterstack.push_back( SegmenterStackElement( m_start_position, m_curr_position_ofs, m_segmenter));
								m_segmenter = ns;
								m_curr_position_ofs = m_curr_position;
								m_segmenter->putInput( segsrc, segsrcsize, true);
							}
						}
						//... start or end of document marker
						else if (featidx == SubDocumentEnd)
						{
							if (m_nof_segments == 0)
							{
								DEBUG_CLOSE();
								return false;
							}
							//... end of sub document -> out of loop and return document
							m_nof_segments = 0;
							completeDocumentProcessing( doc);
							DEBUG_EVENT1( "subdocument-end", "%s", m_subdocTypeName.c_str());
							m_subdocTypeName.clear();
							DEBUG_CLOSE();
							return true;
						}
						else
						{
							if (m_nof_segments > 0)
							{
								throw strus::runtime_error( "%s", _TXT("addressing segments outside of a sub document or overlapping sub documents found"));
							}
							// create new sub document:
							m_subdocTypeName = m_analyzer->subdoctypes()[ featidx-OfsSubDocument];
							m_start_position = m_curr_position;
							DEBUG_EVENT1( "subdocument-start", "%s", m_subdocTypeName.c_str());
						}
					}
					else
					{
						++m_nof_segments;
						if (featidx >= OfsPatternMatchSegment)
						{
							DEBUG_OPEN( "pre-patternmatch");
							PreProcPatternMatchContext& ppctx
								= m_preProcPatternMatchContextMap.context( featidx - OfsPatternMatchSegment);
							std::size_t rel_position = (std::size_t)(m_curr_position - m_start_position);
							ppctx.process( rel_position, segsrc, segsrcsize);
							DEBUG_CLOSE();
						}
						else
						{
							const FeatureConfig& feat = m_analyzer->featureConfigMap().featureConfig( featidx);
							if (feat.tokenizer()->concatBeforeTokenize())
							{
								DEBUG_EVENT1( "concat-feat", "%s", feat.name().c_str());
								// concat chunks that need to be concatenated before tokenization:
								std::size_t rel_position = (std::size_t)(m_curr_position - m_start_position);
								m_segmentProcessor.concatDocumentSegment(
										featidx, rel_position, segsrc, segsrcsize);
							}
							else
							{
								DEBUG_EVENT1( "process-feat", "%s", feat.name().c_str());
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
			if (m_segmenterstack.empty())
			{
				break;
			}
			else
			{
				delete m_segmenter;
				m_segmenter = m_segmenterstack.back().segmenter;
				m_curr_position_ofs = m_segmenterstack.back().curr_position_ofs;
				m_start_position = m_segmenterstack.back().start_position;
				m_segmenterstack.pop_back();
			}
		}
		if (m_eof && m_nof_segments > 0)
		{
			if (!m_subdocTypeName.empty())
			{
				throw strus::runtime_error( _TXT( "sub document '%s' not terminated"), m_subdocTypeName.c_str());
			}
			completeDocumentProcessing( doc);
			m_nof_segments = 0;
			DEBUG_CLOSE();
			return true;
		}
		DEBUG_CLOSE();
		return false;
	}
	CATCH_ERROR_MAP_RETURN( _TXT("error in DocumentAnalyzerContext::analyzeNext: %s"), *m_errorhnd, false);
}

