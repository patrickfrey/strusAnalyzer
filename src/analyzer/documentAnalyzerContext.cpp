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
#include "strus/base/string_conv.hpp"
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

DocumentAnalyzerContext::DocumentAnalyzerContext( const DocumentAnalyzerInstance* analyzer_, const analyzer::DocumentClass& dclass, ErrorBufferInterface* errorhnd_)
	:m_segmentProcessor(analyzer_->featureConfigMap(),analyzer_->patternFeatureConfigMap(), errorhnd_)
	,m_preProcPatternMatchContextMap(analyzer_->preProcPatternMatchConfigMap(), errorhnd_)
	,m_postProcPatternMatchContextMap(analyzer_->postProcPatternMatchConfigMap(), errorhnd_)
	,m_analyzer(analyzer_)
	,m_segmenter(m_analyzer->segmenter()->createContext( dclass))
	,m_segmenterstack()
	,m_eof(false)
	,m_curr_position_ofs(0)
	,m_curr_position(0)
	,m_start_position(0)
	,m_nof_segments(0)
	,m_subdocTypeName()
	,m_activeFields()
	,m_structures()
	,m_errorhnd(errorhnd_)
	,m_debugtrace(0)
{
	if (!m_segmenter)
	{
		throw std::runtime_error( _TXT("failed to create document analyzer context"));
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
	if (m_debugtrace) delete m_debugtrace;
}

void DocumentAnalyzerContext::putInput( const char* chunk, std::size_t chunksize, bool eof)
{
	m_segmenter->putInput( chunk, chunksize, eof);
	m_eof = eof;
}

void DocumentAnalyzerContext::processAggregatedMetadata( analyzer::Document& res) const
{
	std::vector<DocumentAnalyzerInstance::StatisticsConfig>::const_iterator
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
	m_segmentProcessor.eliminateCovered();

	DEBUG_OPEN("patternmatch")
	// fetch pre processing pattern outputs:
	PreProcPatternMatchContextMap::iterator
		vi = m_preProcPatternMatchContextMap.begin(),
		ve = m_preProcPatternMatchContextMap.end();
	for (; vi != ve; ++vi)
	{
		m_segmentProcessor.processPatternMatchResult( (*vi)->fetchResults());
	}

	// process post processing patterns:
	PostProcPatternMatchContextMap::iterator
		pi = m_postProcPatternMatchContextMap.begin(),
		pe = m_postProcPatternMatchContextMap.end();
	for (; pi != pe; ++pi)
	{
		std::vector<std::string> lexems = (*pi)->feeder()->lexemTypes();
		std::vector<std::string>::const_iterator li, le = lexems.end();
		for (li = lexems.begin(); li != le; ++li)
		{
			if (m_analyzer->searchIndexTermTypeSet().find(*li) != m_analyzer->searchIndexTermTypeSet().end()) break;
		}
		if (li != le) (*pi)->process( m_segmentProcessor.searchTerms());

		for (li = lexems.begin(); li != le; ++li)
		{
			if (m_analyzer->forwardIndexTermTypeSet().find(*li) != m_analyzer->forwardIndexTermTypeSet().end()) break;
		}
		if (li != le) (*pi)->process( m_segmentProcessor.forwardTerms());

		(*pi)->process( m_segmentProcessor.patternLexemTerms());
		m_segmentProcessor.processPatternMatchResult( (*pi)->fetchResults());
	}
	DEBUG_CLOSE()

	// create output (with real positions):
	res = m_segmentProcessor.fetchDocument( m_analyzer->structureConfigList(), m_structures);
	res.setSubDocumentTypeName( m_subdocTypeName);

	// Map aggregated metadata, if defined:
	processAggregatedMetadata( res);

	// Reset current document processing state:
	m_segmentProcessor.clearTermMaps();
	pi = m_postProcPatternMatchContextMap.begin();
	for (; pi != pe; ++pi)
	{
		(*pi)->clear();
	}
	vi = m_preProcPatternMatchContextMap.begin();
	for (; vi != ve; ++vi)
	{
		(*vi)->clear();
	}
}

struct FieldArea
{
	enum Type {HeaderType,ContentType};
	Type type;
	SearchIndexStructure::PositionRange positionRange;

	FieldArea()
		:type(ContentType),positionRange(){}
	FieldArea( Type type_, const SearchIndexStructure::PositionRange& positionRange_)
		:type(type_),positionRange(positionRange_){}
	FieldArea( const FieldArea& o)
		:type(o.type),positionRange(o.positionRange){}

	bool operator<( const FieldArea& o) const
	{
		if (positionRange.second == o.positionRange.second)
		{
			return (positionRange.first == o.positionRange.first)
				? (int)type < (int)o.type
				: positionRange.first < o.positionRange.first;
		}
		else
		{
			return positionRange.second < o.positionRange.second;
		}
	}

	bool covers( const FieldArea& oth) const
	{
		return positionRange.second >= oth.positionRange.second
			&& positionRange.first <= oth.positionRange.first;
	}
	bool isequal( const FieldArea& oth) const
	{
		return positionRange.second == oth.positionRange.second
			&& positionRange.first == oth.positionRange.first;
	}
};

static void collectHeaderFields(
	std::set<FieldArea>& res,
	const std::vector<SeachIndexFieldConfig>& fieldConfigs,
	const std::vector<SearchIndexField>& fields, int configIdx)
{
	std::vector<SearchIndexField>::const_iterator fi = fields.begin(), fe = fields.end();
	for (; fi != fe; ++fi)
	{
		const SeachIndexFieldConfig& config = fieldConfigs[ fi->configIdx()];
		std::vector<int>::const_iterator
			hi = config.headerStructureList().begin(),
			he = config.headerStructureList().end();
		for (; hi != he; ++hi)
		{
			if (*hi == configIdx)
			{
				res.insert( FieldArea(
					FieldArea::HeaderType,
					SearchIndexStructure::PositionRange( fi->start(), fi->end())));
			}
		}
	}
}

static void collectContentFields(
	std::set<FieldArea>& res,
	const std::vector<SeachIndexFieldConfig>& fieldConfigs,
	const std::vector<SearchIndexField>& fields, int configIdx)
{
	std::vector<SearchIndexField>::const_iterator fi = fields.begin(), fe = fields.end();
	for (; fi != fe; ++fi)
	{
		const SeachIndexFieldConfig& config = fieldConfigs[ fi->configIdx()];
		std::vector<int>::const_iterator
			ci = config.contentStructureList().begin(),
			ce = config.contentStructureList().end();
		for (; ci != ce; ++ci)
		{
			if (*ci == configIdx)
			{
				res.insert( FieldArea(
					FieldArea::ContentType,
					SearchIndexStructure::PositionRange( fi->start(), fi->end())));
			}
		}
	}
}

static void addStructureFieldCandidate( std::vector<FieldArea>& candidates, const FieldArea& area)
{
	std::vector<FieldArea>::iterator ci = candidates.begin(), ce = candidates.end();
	while (ci != ce)
	{
		if (ci->covers( area))
		{
			return;
		}
		else if (area.covers( *ci))
		{
			ci = candidates.erase( ci);
			ce = candidates.end();
		}
		else
		{
			++ci;
		}
	}
	candidates.push_back( area);
}

static void addInnerStructureFieldCandidate( std::vector<FieldArea>& candidates, const FieldArea& area)
{
	std::vector<FieldArea>::iterator ci = candidates.begin(), ce = candidates.end();
	while (ci != ce)
	{
		if (ci->covers( area))
		{
			ci = candidates.erase( ci);
			ce = candidates.end();
		}
		else if (area.covers( *ci))
		{
			return;
		}
		else
		{
			++ci;
		}
	}
	candidates.push_back( area);
}

static void flushStructureFieldCandidates( std::vector<SearchIndexStructure>& dest, int configIdx, const FieldArea& header, const std::vector<FieldArea>& candidates)
{
	std::vector<FieldArea>::const_iterator ci = candidates.begin(), ce = candidates.end();
	for (; ci != ce; ++ci)
	{
		dest.push_back( SearchIndexStructure( configIdx, header.positionRange, ci->positionRange));
	}
}

void DocumentAnalyzerContext::buildStructures( const std::vector<SearchIndexField>& fields, int configIdx)
{
	const SeachIndexStructureConfig& structConfig
		= m_analyzer->structureConfigList()[ configIdx];
	std::set<FieldArea> areaset;

	collectHeaderFields( areaset, m_analyzer->fieldConfigList(), fields, configIdx);
	collectContentFields( areaset, m_analyzer->fieldConfigList(), fields, configIdx);

	switch (structConfig.structureType())
	{
		case DocumentAnalyzerInstanceInterface::StructureCover:
		{
			// Find header covering content completely (not equal)
			std::set<FieldArea>::reverse_iterator ai = areaset.rbegin(), ae = areaset.rend();
			for (; ai != ae; ++ai)
			{
				if (ai->type == FieldArea::HeaderType)
				{
					std::vector<FieldArea> candidates;
					std::set<FieldArea>::reverse_iterator next_ai = ai;
					++next_ai;
					for (; next_ai != ae; ++next_ai)
					{
						if (next_ai->type == FieldArea::ContentType
						&&  ai->covers( *next_ai)
						&& !ai->isequal( *next_ai))
						{
							addStructureFieldCandidate( candidates, *next_ai);
						}
					}
					flushStructureFieldCandidates( m_structures, configIdx, *ai, candidates);
				}
			}
			break;
		}
		case DocumentAnalyzerInstanceInterface::StructureLabel:
		{
			// Find header inside content (covered by content, not equal)
			std::set<FieldArea>::const_iterator ai = areaset.begin(), ae = areaset.end();
			for (; ai != ae; ++ai)
			{
				if (ai->type == FieldArea::HeaderType)
				{
					std::vector<FieldArea> candidates;
					std::set<FieldArea>::const_iterator next_ai = ai;
					++next_ai;
					for (; next_ai != ae; ++next_ai)
					{
						if (next_ai->type == FieldArea::ContentType
						&&  next_ai->covers( *ai)
						&& !next_ai->isequal( *ai))
						{
							addInnerStructureFieldCandidate( candidates, *next_ai);
						}
					}
					flushStructureFieldCandidates( m_structures, configIdx, *ai, candidates);
				}
			}
		}
		case DocumentAnalyzerInstanceInterface::StructureHeader:
		{
			std::set<FieldArea>::const_iterator ai = areaset.begin(), ae = areaset.end();
			for (; ai != ae; ++ai)
			{
				if (ai->type == FieldArea::HeaderType)
				{
					std::vector<FieldArea> candidates;
					std::set<FieldArea>::const_iterator next_ai = ai;
					++next_ai;
					for (; next_ai != ae && next_ai->type == FieldArea::ContentType; ++next_ai)
					{
						if (next_ai->positionRange.first >= ai->positionRange.second)
						{
							addStructureFieldCandidate( candidates, *next_ai);
						}
					}
					flushStructureFieldCandidates( m_structures, configIdx, *ai, candidates);
				}
			}
			break;
		}
		case DocumentAnalyzerInstanceInterface::StructureFooter:
		{
			std::set<FieldArea>::reverse_iterator ai = areaset.rbegin(), ae = areaset.rend();
			for (; ai != ae; ++ai)
			{
				if (ai->type == FieldArea::HeaderType)
				{
					std::vector<FieldArea> candidates;
					std::set<FieldArea>::reverse_iterator next_ai = ai;
					++next_ai;
					for (; next_ai != ae && next_ai->type == FieldArea::ContentType; ++next_ai)
					{
						if (next_ai->positionRange.second <= ai->positionRange.first)
						{
							addStructureFieldCandidate( candidates, *next_ai);
						}
					}
					flushStructureFieldCandidates( m_structures, configIdx, *ai, candidates);
				}
			}
			break;
		}
	}
}

void DocumentAnalyzerContext::collectIndexFields( int scopeIdx)
{
	std::vector<SearchIndexStructure> new_structures;
	std::vector<SearchIndexField> otherFields;
	std::vector<SearchIndexField> selectedFields;
	std::set<int> candidateStructures;

	// [1] Divide active field in 2 parts: to process 'selectedFields', to keep 'otherFields'
	//	and find candidate structures to build:
	std::vector<SearchIndexField>::iterator ai = m_activeFields.begin(), ae = m_activeFields.end();
	for (; ai != ae; ++ai)
	{
		if (ai->scopeIdx() == scopeIdx)
		{
			selectedFields.push_back( *ai);

			const SeachIndexFieldConfig& config = m_analyzer->fieldConfigList()[ ai->configIdx()];
			const std::vector<int>& headerRefs = config.headerStructureList();
			std::vector<int>::const_iterator hi = headerRefs.begin(), he = headerRefs.end();
			for (; hi != he; ++hi)
			{
				candidateStructures.insert( *hi);
			}
		}
		else
		{
			otherFields.push_back( *ai);
		}
	}
	// [2] Build structures:
	std::set<int>::const_iterator ci = candidateStructures.begin(), ce = candidateStructures.end();
	for (; ci != ce; ++ci)
	{
		buildStructures( selectedFields, *ci);
	}
	// [3] Remove fields processed from active fields:
	std::swap( m_activeFields, otherFields);
}

void DocumentAnalyzerContext::handleStructureEvent( int evhnd, const char* segsrc, std::size_t segsize)
{
	int fidx = FieldEventIdx( evhnd);

	switch ((FieldEvent)FieldEventType(evhnd))
	{
		case FieldEvent_Collect:
		{
			int scopeIdx = fidx;
			collectIndexFields( scopeIdx);
			break;
		}
		case FieldEvent_Id:
		{
			int configIdx = fidx;
			std::size_t aidx = m_activeFields.size();
			for (; aidx && (m_activeFields[aidx-1].configIdx() != configIdx || m_activeFields[aidx-1].end().defined()); --aidx){}
			if (!aidx) throw std::runtime_error(_TXT("logic error: field id event without start detected"));
			m_activeFields[aidx-1].setId( strus::string_conv::trim( segsrc, segsize));
			break;
		}
		case FieldEvent_Start:
		{
			int configIdx = fidx;
			const SeachIndexFieldConfig& fieldConfig = m_analyzer->fieldConfigList()[ configIdx];
			m_activeFields.push_back( SearchIndexField( configIdx, fieldConfig.scopeIdx()));
			m_activeFields.back().setStart( analyzer::Position( (int)m_curr_position, 0));
			break;
		}
		case FieldEvent_End:
		{
			int configIdx = fidx;
			std::size_t aidx = m_activeFields.size();
			for (; aidx && (m_activeFields[aidx-1].configIdx() != configIdx || m_activeFields[aidx-1].end().defined()); --aidx){}
			if (!aidx) throw std::runtime_error(_TXT("logic error: end of field event without start detected"));
			m_activeFields[aidx-1].setEnd( analyzer::Position( (int)m_curr_position, 0));
			break;
		}
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
							const DocumentAnalyzerInstance::SubSegmenterDef* subsegmenterdef = m_analyzer->subsegmenter( featidx - OfsSubContent);
							if (subsegmenterdef)
							{
								DEBUG_EVENT2( "subcontent", "%s; charset=%s", subsegmenterdef->documentClass.mimeType().c_str(), subsegmenterdef->documentClass.encoding().c_str());
								SegmenterContextInterface* ns = subsegmenterdef->segmenterInstance->createContext( subsegmenterdef->documentClass);
								if (!ns) throw std::runtime_error( _TXT("failed to create sub segmenter context"));
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
								throw std::runtime_error( _TXT("addressing segments outside of a sub document or overlapping sub documents found"));
							}
							// create new sub document:
							m_subdocTypeName = m_analyzer->subdoctypes()[ featidx-OfsSubDocument];
							m_start_position = m_curr_position;
							DEBUG_EVENT1( "subdocument-start", "%s", m_subdocTypeName.c_str());
						}
					}
					else if (featidx >= OfsStructureElement)
					{
						int evhnd = featidx - OfsStructureElement;
						handleStructureEvent( evhnd, segsrc, segsrcsize);
					}
					else
					{
						// Features:
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

