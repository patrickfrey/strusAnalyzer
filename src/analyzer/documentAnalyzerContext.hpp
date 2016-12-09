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
#include "patternFeatureContextMap.hpp"
#include "strus/documentAnalyzerContextInterface.hpp"
#include "strus/segmenterContextInterface.hpp"

namespace strus
{
/// \brief Forward declaration
class ErrorBufferInterface;


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
	void mapStatistics( analyzer::Document& res) const;
	void processPatternMatchResult( const std::vector<BindTerm>& result);

private:
	SegmentProcessor m_segmentProcessor;
	PreProcPatternMatchContextMap m_preProcPatternMatchContextMap;
	PostProcPatternMatchContextMap m_postProcPatternMatchContextMap;
	const DocumentAnalyzer* m_analyzer;
	SegmenterContextInterface* m_segmenter;
	std::vector<analyzer::Document> m_subdocstack;
	bool m_eof;
	SegmenterPosition m_curr_position;
	SegmenterPosition m_start_position;
	ErrorBufferInterface* m_errorhnd;
};

}//namespace
#endif

