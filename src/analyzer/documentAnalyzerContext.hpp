/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef _STRUS_DOCUMENT_ANALYZER_CONTEXT_HPP_INCLUDED
#define _STRUS_DOCUMENT_ANALYZER_CONTEXT_HPP_INCLUDED
#include "documentAnalyzer.hpp"
#include "segmentProcessor.hpp"
#include "patternMatchContextMap.hpp"
#include "patternFeatureConfigMap.hpp"
#include "strus/documentAnalyzerContextInterface.hpp"
#include "strus/segmenterContextInterface.hpp"

namespace strus
{
/// \brief Forward declaration
class ErrorBufferInterface;
/// \brief Forward declaration
class DebugTraceContextInterface;

class DocumentAnalyzerContext
	:public DocumentAnalyzerContextInterface
{
public:
	DocumentAnalyzerContext(
			const DocumentAnalyzer* analyzer_,
			const analyzer::DocumentClass& dclass,
			ErrorBufferInterface* errorhnd);
	virtual ~DocumentAnalyzerContext();

	virtual void putInput(const char* chunk, std::size_t chunksize, bool eof);

	virtual bool analyzeNext( analyzer::Document& doc);

private:
	void processAggregatedMetadata( analyzer::Document& res) const;
	void processPatternMatchResult( const std::vector<BindTerm>& result);
	void completeDocumentProcessing( analyzer::Document& res);

	struct SegmenterStackElement
	{
		SegmenterPosition start_position;
		SegmenterPosition curr_position_ofs;
		SegmenterContextInterface* segmenter;

		SegmenterStackElement( SegmenterPosition start_position_, SegmenterPosition curr_position_ofs_, SegmenterContextInterface* segmenter_)
			:start_position(start_position_),curr_position_ofs(curr_position_ofs_),segmenter(segmenter_){}
		SegmenterStackElement( const SegmenterStackElement& o)
			:start_position(o.start_position),curr_position_ofs(o.curr_position_ofs),segmenter(o.segmenter){}
	};

private:
	SegmentProcessor m_segmentProcessor;
	PreProcPatternMatchContextMap m_preProcPatternMatchContextMap;
	PostProcPatternMatchContextMap m_postProcPatternMatchContextMap;
	const DocumentAnalyzer* m_analyzer;
	SegmenterContextInterface* m_segmenter;
	std::vector<SegmenterStackElement> m_segmenterstack;
	bool m_eof;
	SegmenterPosition m_curr_position_ofs;
	SegmenterPosition m_curr_position;
	SegmenterPosition m_start_position;
	unsigned int m_nof_segments;
	std::string m_subdocTypeName;
	ErrorBufferInterface* m_errorhnd;
	DebugTraceContextInterface* m_debugtrace;
};

}//namespace
#endif

