/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "documentAnalyzer.hpp"
#include "documentAnalyzerContext.hpp"
#include "strus/normalizerFunctionInstanceInterface.hpp"
#include "strus/tokenizerFunctionInstanceInterface.hpp"
#include "strus/segmenterInstanceInterface.hpp"
#include "strus/segmenterInterface.hpp"
#include "strus/textProcessorInterface.hpp"
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

DocumentAnalyzer::DocumentAnalyzer( const TextProcessorInterface* textproc_, const SegmenterInterface* segmenter_, const analyzer::SegmenterOptions& opts, ErrorBufferInterface* errorhnd)
	:m_textproc(textproc_)
	,m_segmenter(segmenter_->createInstance( opts))
	,m_subsegmenterList()
	,m_featureConfigMap()
	,m_preProcPatternMatchConfigMap()
	,m_postProcPatternMatchConfigMap()
	,m_patternFeatureConfigMap()
	,m_subdoctypear()
	,m_statistics()
	,m_forwardIndexTermTypeSet()
	,m_searchIndexTermTypeSet()
	,m_errorhnd(errorhnd)
{
	if (!m_segmenter)
	{
		throw strus::runtime_error( _TXT("failed to create segmenter context: %s"), errorhnd->fetchError());
	}
}

DocumentAnalyzer::~DocumentAnalyzer()
{
	delete m_segmenter;
	std::vector<SubSegmenterDef>::iterator si = m_subsegmenterList.begin(), se = m_subsegmenterList.end();
	for (; si != se; ++si)
	{
		delete si->segmenterInstance;
	}
}

static int getSubSegmenterIndex( const std::vector<DocumentAnalyzer::SubSegmenterDef>& segmenterList, const std::string& selectexpr)
{
	int rt = -1;
	std::size_t maxlen = 0;

	std::vector<DocumentAnalyzer::SubSegmenterDef>::const_iterator si = segmenterList.begin(), se = segmenterList.end();
	for (int sidx=0; si != se; ++si,++sidx)
	{
		if (si->selectorPrefix.size() <= selectexpr.size()
			&& si->selectorPrefix.size() > maxlen
			&& 0==std::memcmp( selectexpr.c_str(), si->selectorPrefix.c_str(), si->selectorPrefix.size()))
		{
			maxlen = si->selectorPrefix.size();
			rt = sidx;
		}
	}
	return rt;
}

void DocumentAnalyzer::defineSelectorExpression( unsigned int featidx, const std::string& selectexpr)
{
	int sidx = getSubSegmenterIndex( m_subsegmenterList, selectexpr);
	if (sidx >= 0)
	{
		m_subsegmenterList[ sidx].segmenterInstance->defineSelectorExpression( featidx, selectexpr.c_str() + m_subsegmenterList[sidx].selectorPrefix.size());
	}
	else
	{
		m_segmenter->defineSelectorExpression( featidx, selectexpr);
	}
}

void DocumentAnalyzer::defineSubSection( int startId, int endId, const std::string& selectexpr)
{
	int sidx = getSubSegmenterIndex( m_subsegmenterList, selectexpr);
	if (sidx >= 0)
	{
		m_subsegmenterList[ sidx].segmenterInstance->defineSubSection( startId, endId, selectexpr.c_str() + m_subsegmenterList[sidx].selectorPrefix.size());
	}
	else
	{
		m_segmenter->defineSubSection( startId, endId, selectexpr);
	}
}

void DocumentAnalyzer::addSearchIndexFeature(
		const std::string& type,
		const std::string& selectexpr,
		TokenizerFunctionInstanceInterface* tokenizer,
		const std::vector<NormalizerFunctionInstanceInterface*>& normalizers,
		const analyzer::FeatureOptions& options)
{
	try
	{
		unsigned int featidx = m_featureConfigMap.defineFeature( FeatSearchIndexTerm, type, tokenizer, normalizers, options);
		defineSelectorExpression( featidx, selectexpr);
		m_searchIndexTermTypeSet.insert( utils::tolower( type));
	}
	CATCH_ERROR_MAP( _TXT("error adding search index feature: %s"), *m_errorhnd);
}

void DocumentAnalyzer::addForwardIndexFeature(
		const std::string& type,
		const std::string& selectexpr,
		TokenizerFunctionInstanceInterface* tokenizer,
		const std::vector<NormalizerFunctionInstanceInterface*>& normalizers,
		const analyzer::FeatureOptions& options)
{
	try
	{
		unsigned int featidx = m_featureConfigMap.defineFeature( FeatForwardIndexTerm, type, tokenizer, normalizers, options);
		defineSelectorExpression( featidx, selectexpr);
		m_forwardIndexTermTypeSet.insert( utils::tolower( type));
	}
	CATCH_ERROR_MAP( _TXT("error adding forward index feature: %s"), *m_errorhnd);
}

void DocumentAnalyzer::defineMetaData(
		const std::string& metaname,
		const std::string& selectexpr,
		TokenizerFunctionInstanceInterface* tokenizer,
		const std::vector<NormalizerFunctionInstanceInterface*>& normalizers)
{
	try
	{
		unsigned int featidx = m_featureConfigMap.defineFeature( FeatMetaData, metaname, tokenizer, normalizers, analyzer::FeatureOptions());
		defineSelectorExpression( featidx, selectexpr);
	}
	CATCH_ERROR_MAP( _TXT("error defining metadata: %s"), *m_errorhnd);
}

void DocumentAnalyzer::defineAttribute(
		const std::string& attribname,
		const std::string& selectexpr,
		TokenizerFunctionInstanceInterface* tokenizer,
		const std::vector<NormalizerFunctionInstanceInterface*>& normalizers)
{
	try
	{
		unsigned int featidx = m_featureConfigMap.defineFeature( FeatAttribute, attribname, tokenizer, normalizers, analyzer::FeatureOptions());
		defineSelectorExpression( featidx, selectexpr);
	}
	CATCH_ERROR_MAP( _TXT("error defining attribute: %s"), *m_errorhnd);
}

void DocumentAnalyzer::defineAggregatedMetaData(
		const std::string& metaname,
		AggregatorFunctionInstanceInterface* statfunc)
{
	try
	{
		// PF:NOTE: The following order of code ensures that if this constructor fails statfunc is not copied and can be freed by this function:
		m_statistics.reserve( m_statistics.size()+1);
		m_statistics.push_back( StatisticsConfig( metaname, statfunc));
	}
	CATCH_ERROR_MAP( _TXT("error defining aggregated metadata: %s"), *m_errorhnd);
}

void DocumentAnalyzer::defineSubDocument(
		const std::string& subDocumentTypeName,
		const std::string& selectexpr)
{
	try
	{
		unsigned int subDocumentType = m_subdoctypear.size();
		m_subdoctypear.push_back( subDocumentTypeName);
		if (subDocumentType >= MaxNofSubDocuments)
		{
			throw strus::runtime_error( _TXT("too many sub documents defined"));
		}
		defineSubSection( subDocumentType+OfsSubDocument, SubDocumentEnd, selectexpr);
	}
	CATCH_ERROR_MAP( _TXT("error in DocumentAnalyzer::defineSubDocument: %s"), *m_errorhnd);
}

void DocumentAnalyzer::defineSubContent(
		const std::string& selectexpr,
		const analyzer::DocumentClass& documentClass)
{
	try
	{
		defineSelectorExpression( m_subsegmenterList.size()+OfsSubContent, selectexpr);
		const SegmenterInterface* segmenter = m_textproc->getSegmenterByMimeType( documentClass.mimeType());
		if (!segmenter)
		{
			throw strus::runtime_error(_TXT("no document segmenter defined for encoding=%s; mimetype=%s; scheme=%s"), documentClass.encoding().c_str(), documentClass.mimeType().c_str(), documentClass.scheme().c_str());
		}
		analyzer::SegmenterOptions opts;
		if (!documentClass.scheme().empty())
		{
			opts = m_textproc->getSegmenterOptions( documentClass.scheme());
		}
		Reference<SegmenterInstanceInterface> segmenterinst( segmenter->createInstance( opts));
		if (!segmenterinst.get()) throw strus::runtime_error(_TXT("failed to create segmenter instance"));
		m_subsegmenterList.push_back( SubSegmenterDef( documentClass, segmenterinst.get(), selectexpr));
		segmenterinst.release();
	}
	CATCH_ERROR_MAP( _TXT("error in DocumentAnalyzer::defineSubContentSegmenter: %s"), *m_errorhnd);
}

void DocumentAnalyzer::addPatternLexem(
		const std::string& termtype,
		const std::string& selectexpr,
		TokenizerFunctionInstanceInterface* tokenizer,
		const std::vector<NormalizerFunctionInstanceInterface*>& normalizers)
{
	unsigned int featidx = m_featureConfigMap.defineFeature( FeatPatternLexem, termtype, tokenizer, normalizers, analyzer::FeatureOptions());
	defineSelectorExpression( featidx, selectexpr);
}

void DocumentAnalyzer::definePatternMatcherPostProc(
		const std::string& patternTypeName,
		PatternMatcherInstanceInterface* matcher,
		PatternTermFeederInstanceInterface* feeder)
{
	try
	{
		(void)m_postProcPatternMatchConfigMap.definePatternMatcher( patternTypeName, matcher, feeder, true);
	}
	CATCH_ERROR_MAP( _TXT("error defining post processing pattern match: %s"), *m_errorhnd);
}

void DocumentAnalyzer::definePatternMatcherPreProc(
		const std::string& patternTypeName,
		PatternMatcherInstanceInterface* matcher,
		PatternLexerInstanceInterface* lexer,
		const std::vector<std::string>& selectexpr)
{
	try
	{
		unsigned int idx = OfsPatternMatchSegment
				+ m_preProcPatternMatchConfigMap.definePatternMatcher( patternTypeName, matcher, lexer, true);
		std::vector<std::string>::const_iterator
			si = selectexpr.begin(), se = selectexpr.end();
		for (; si != se; ++si)
		{
			defineSelectorExpression( idx, *si);
		}
	}
	CATCH_ERROR_MAP( _TXT("error defining pre processing pattern match: %s"), *m_errorhnd);
}

void DocumentAnalyzer::addSearchIndexFeatureFromPatternMatch(
		const std::string& type,
		const std::string& patternTypeName,
		const std::vector<NormalizerFunctionInstanceInterface*>& normalizers,
		const analyzer::FeatureOptions& options)
{
	try
	{
		m_patternFeatureConfigMap.defineFeature( FeatSearchIndexTerm, type, patternTypeName, normalizers, options);
	}
	CATCH_ERROR_MAP( _TXT("error defining search index feature from pattern matching result: %s"), *m_errorhnd);
}

void DocumentAnalyzer::addForwardIndexFeatureFromPatternMatch(
		const std::string& type,
		const std::string& patternTypeName,
		const std::vector<NormalizerFunctionInstanceInterface*>& normalizers,
		const analyzer::FeatureOptions& options)
{
	try
	{
		m_patternFeatureConfigMap.defineFeature( FeatForwardIndexTerm, type, patternTypeName, normalizers, options);
	}
	CATCH_ERROR_MAP( _TXT("error defining forward index feature from pattern matching result: %s"), *m_errorhnd);
}

void DocumentAnalyzer::defineMetaDataFromPatternMatch(
		const std::string& metaname,
		const std::string& patternTypeName,
		const std::vector<NormalizerFunctionInstanceInterface*>& normalizers)
{
	try
	{
		m_patternFeatureConfigMap.defineFeature( FeatMetaData, metaname, patternTypeName, normalizers, analyzer::FeatureOptions());
	}
	CATCH_ERROR_MAP( _TXT("error defining document meta data from pattern matching result: %s"), *m_errorhnd);
}

void DocumentAnalyzer::defineAttributeFromPatternMatch(
		const std::string& attribname,
		const std::string& patternTypeName,
		const std::vector<NormalizerFunctionInstanceInterface*>& normalizers)
{
	try
	{
		m_patternFeatureConfigMap.defineFeature( FeatAttribute, attribname, patternTypeName, normalizers, analyzer::FeatureOptions());
	}
	CATCH_ERROR_MAP( _TXT("error defining document attribute from pattern matching result: %s"), *m_errorhnd);
}

analyzer::Document DocumentAnalyzer::analyze(
		const std::string& content,
		const analyzer::DocumentClass& dclass) const
{
	try
	{
		analyzer::Document rt;
		std::auto_ptr<DocumentAnalyzerContext>
			analyzerInstance( new DocumentAnalyzerContext( this, dclass, m_errorhnd));
		analyzerInstance->putInput( content.c_str(), content.size(), true);
		if (!analyzerInstance->analyzeNext( rt))
		{
			throw strus::runtime_error( _TXT("analyzed content incomplete or empty"));
		}
		return rt;
	}
	CATCH_ERROR_MAP_RETURN( _TXT("error in document analyze: %s"), *m_errorhnd, analyzer::Document());
}

DocumentAnalyzerContextInterface* DocumentAnalyzer::createContext( const analyzer::DocumentClass& dclass) const
{
	try
	{
		return new DocumentAnalyzerContext( this, dclass, m_errorhnd);
	}
	CATCH_ERROR_MAP_RETURN( _TXT("error in document analyzer create context: %s"), *m_errorhnd, 0);
}
