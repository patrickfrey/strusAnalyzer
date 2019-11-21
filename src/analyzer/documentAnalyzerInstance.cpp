/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "documentAnalyzerInstance.hpp"
#include "documentAnalyzerContext.hpp"
#include "strus/normalizerFunctionInstanceInterface.hpp"
#include "strus/tokenizerFunctionInstanceInterface.hpp"
#include "strus/segmenterInstanceInterface.hpp"
#include "strus/segmenterInterface.hpp"
#include "strus/textProcessorInterface.hpp"
#include "strus/errorBufferInterface.hpp"
#include "strus/base/local_ptr.hpp"
#include "strus/base/string_conv.hpp"
#include "private/subDocumentDefinitionView.hpp"
#include "private/featureView.hpp"
#include "private/errorUtils.hpp"
#include "private/internationalization.hpp"
#include "private/xpath.hpp"
#include <stdexcept>
#include <sstream>
#include <iostream>
#include <set>
#include <map>
#include <cstring>
#include <limits>

using namespace strus;

DocumentAnalyzerInstance::DocumentAnalyzerInstance( const TextProcessorInterface* textproc_, const SegmenterInterface* segmenter_, const analyzer::SegmenterOptions& opts, ErrorBufferInterface* errorhnd)
	:m_textproc(textproc_)
	,m_segmenter(segmenter_->createInstance( opts))
	,m_subDocumentList()
	,m_subsegmenterList()
	,m_featureConfigMap()
	,m_fieldConfigList()
	,m_structureConfigList()
	,m_structureScopeMap()
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

DocumentAnalyzerInstance::~DocumentAnalyzerInstance()
{
	delete m_segmenter;
	std::vector<SubSegmenterDef>::iterator si = m_subsegmenterList.begin(), se = m_subsegmenterList.end();
	for (; si != se; ++si)
	{
		delete si->segmenterInstance;
	}
}

static int getSubSegmenterIndex( const std::vector<DocumentAnalyzerInstance::SubSegmenterDef>& segmenterList, const std::string& selectexpr)
{
	int rt = -1;
	std::size_t maxlen = 0;

	std::vector<DocumentAnalyzerInstance::SubSegmenterDef>::const_iterator si = segmenterList.begin(), se = segmenterList.end();
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

void DocumentAnalyzerInstance::defineSelectorExpression( unsigned int featidx, const std::string& selectexpr)
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

void DocumentAnalyzerInstance::defineSubSection( int startId, int endId, const std::string& selectexpr)
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

void DocumentAnalyzerInstance::addSearchIndexFeature(
		const std::string& type,
		const std::string& selectexpr,
		TokenizerFunctionInstanceInterface* tokenizer,
		const std::vector<NormalizerFunctionInstanceInterface*>& normalizers,
		int priority,
		const analyzer::FeatureOptions& options)
{
	try
	{
		unsigned int featidx = m_featureConfigMap.defineFeature( FeatSearchIndexTerm, type, selectexpr, tokenizer, normalizers, priority, options);
		defineSelectorExpression( featidx, selectexpr);
		m_searchIndexTermTypeSet.insert( string_conv::tolower( type));
	}
	CATCH_ERROR_MAP( _TXT("error adding search index feature: %s"), *m_errorhnd);
}

void DocumentAnalyzerInstance::addForwardIndexFeature(
		const std::string& type,
		const std::string& selectexpr,
		TokenizerFunctionInstanceInterface* tokenizer,
		const std::vector<NormalizerFunctionInstanceInterface*>& normalizers,
		int priority,
		const analyzer::FeatureOptions& options)
{
	try
	{
		unsigned int featidx = m_featureConfigMap.defineFeature( FeatForwardIndexTerm, type, selectexpr, tokenizer, normalizers, priority, options);
		defineSelectorExpression( featidx, selectexpr);
		m_forwardIndexTermTypeSet.insert( string_conv::tolower( type));
	}
	CATCH_ERROR_MAP( _TXT("error adding forward index feature: %s"), *m_errorhnd);
}

void DocumentAnalyzerInstance::addSearchIndexField(
		const std::string& name,
		const std::string& scopeexpr,
		const std::string& selectexpr,
		const std::string& keyexpr)
{
	try
	{
		if (MaxFieldEventIdx <= m_fieldConfigList.size())
		{
			throw strus::runtime_error(_TXT("too many fields for structures defined"));
		}
		std::string full_selectexpr_start = strus::xpathStartStructurePath( strus::joinXPathExpression( scopeexpr, selectexpr));
		// Define XPath selector events of this field:
		unsigned int fidx = m_fieldConfigList.size();
		if (!keyexpr.empty())
		{
			std::string full_keyexpr = strus::joinXPathExpression( scopeexpr, keyexpr);
			if (!stringStartsWith( full_keyexpr, full_selectexpr_start))
			{
				throw strus::runtime_error(_TXT("expression for key has to select content inside the selection expression, but select '%s' not a prefix of key '%s'"), selectexpr.c_str(), keyexpr.c_str());
			}
			defineSelectorExpression(
				OfsStructureElement + FieldEventHandle( FieldEvent_Id, fidx),
				full_keyexpr);
		}
		std::string full_selectexpr_end = strus::xpathEndStructurePath( full_selectexpr_start);
		std::string norm_scopeexpr = strus::xpathEndStructurePath( scopeexpr);
		defineSelectorExpression(
			OfsStructureElement + FieldEventHandle( FieldEvent_Start, fidx),
			full_selectexpr_start);
		defineSelectorExpression(
			OfsStructureElement + FieldEventHandle( FieldEvent_End, fidx),
			full_selectexpr_end);
		int scopeIdx = m_structureScopeMap.size()+1;
		std::pair<StructureScopeMap::iterator,bool> ins
			= m_structureScopeMap.insert(
				StructureScopeMap::value_type( norm_scopeexpr, scopeIdx));
		if (ins.second == true/*insert took place*/)
		{
			defineSelectorExpression(
				OfsStructureElement + FieldEventHandle( FieldEvent_Collect, scopeIdx),
				norm_scopeexpr);
		}
		else
		{
			StructureScopeMap::iterator found = ins.first;
			scopeIdx = found->second;
		}
		// Define the field and its relations to structures referencing it:
		m_fieldConfigList.push_back( 
			SeachIndexFieldConfig(
				string_conv::tolower(name), 
				scopeexpr, selectexpr, keyexpr, scopeIdx));

		std::vector<SeachIndexStructureConfig>::const_iterator si = m_structureConfigList.begin(), se = m_structureConfigList.end();
		for (int sidx=0; si != se; ++si,++sidx)
		{
			if (si->headerName() == m_fieldConfigList.back().name())
			{
				m_fieldConfigList.back().defineHeaderStructureRef( sidx);
			}
			if (si->contentName() == m_fieldConfigList.back().name())
			{
				m_fieldConfigList.back().defineContentStructureRef( sidx);
			}
		}
	}
	CATCH_ERROR_MAP( _TXT("error adding search index field: %s"), *m_errorhnd);
}

void DocumentAnalyzerInstance::addSearchIndexStructure(
		const std::string& name,
		const std::string& headerFieldName,
		const std::string& contentFieldName,
		const StructureType& structureType)
{
	try
	{
		int sidx = m_structureConfigList.size();
		m_structureConfigList.push_back( SeachIndexStructureConfig( string_conv::tolower(name), string_conv::tolower(headerFieldName), string_conv::tolower(contentFieldName), structureType));
		std::vector<SeachIndexFieldConfig>::iterator fi = m_fieldConfigList.begin(), fe = m_fieldConfigList.end();
		for (; fi != fe; ++fi)
		{
			if (fi->name() == m_structureConfigList.back().headerName())
			{
				fi->defineHeaderStructureRef( sidx);
			}
			if (fi->name() == m_structureConfigList.back().contentName())
			{
				fi->defineContentStructureRef( sidx);
			}
		}
	}
	CATCH_ERROR_MAP( _TXT("error adding search index structure: %s"), *m_errorhnd);
}

void DocumentAnalyzerInstance::defineMetaData(
		const std::string& metaname,
		const std::string& selectexpr,
		TokenizerFunctionInstanceInterface* tokenizer,
		const std::vector<NormalizerFunctionInstanceInterface*>& normalizers)
{
	try
	{
		unsigned int featidx = m_featureConfigMap.defineFeature( FeatMetaData, metaname, selectexpr, tokenizer, normalizers, 0/*priority*/, analyzer::FeatureOptions());
		defineSelectorExpression( featidx, selectexpr);
	}
	CATCH_ERROR_MAP( _TXT("error defining metadata: %s"), *m_errorhnd);
}

void DocumentAnalyzerInstance::defineAttribute(
		const std::string& attribname,
		const std::string& selectexpr,
		TokenizerFunctionInstanceInterface* tokenizer,
		const std::vector<NormalizerFunctionInstanceInterface*>& normalizers)
{
	try
	{
		unsigned int featidx = m_featureConfigMap.defineFeature( FeatAttribute, attribname, selectexpr, tokenizer, normalizers, 0/*priority*/, analyzer::FeatureOptions());
		defineSelectorExpression( featidx, selectexpr);
	}
	CATCH_ERROR_MAP( _TXT("error defining attribute: %s"), *m_errorhnd);
}

void DocumentAnalyzerInstance::defineAggregatedMetaData(
		const std::string& metaname,
		AggregatorFunctionInstanceInterface* statfunc)
{
	try
	{
		m_statistics.push_back( StatisticsConfig( metaname, statfunc));
	}
	catch (const std::bad_alloc&)
	{
		m_errorhnd->report( ErrorCodeOutOfMem, _TXT("out of memory defining aggregated metadata '%s'"), metaname.c_str());
		delete statfunc;
	}
}

void DocumentAnalyzerInstance::defineSubDocument(
		const std::string& subDocumentTypeName,
		const std::string& selectexpr)
{
	try
	{
		int subDocumentType = m_subdoctypear.size();
		m_subdoctypear.push_back( subDocumentTypeName);
		m_subDocumentList.push_back( std::pair<std::string,std::string>( subDocumentTypeName, selectexpr));
		if (subDocumentType >= MaxNofSubDocuments)
		{
			throw std::runtime_error( _TXT("too many sub documents defined"));
		}
		defineSubSection( subDocumentType+OfsSubDocument, SubDocumentEnd, selectexpr);
	}
	CATCH_ERROR_MAP( _TXT("error in DocumentAnalyzerInstance::defineSubDocument: %s"), *m_errorhnd);
}

void DocumentAnalyzerInstance::defineSubContent(
		const std::string& selectexpr,
		const analyzer::DocumentClass& documentClass)
{
	try
	{
		defineSelectorExpression( m_subsegmenterList.size()+OfsSubContent, selectexpr);
		const SegmenterInterface* segmenter_ = m_textproc->getSegmenterByMimeType( documentClass.mimeType());
		if (!segmenter_)
		{
			throw strus::runtime_error(_TXT("no document segmenter defined for encoding=%s; mimetype=%s; schema=%s"), documentClass.encoding().c_str(), documentClass.mimeType().c_str(), documentClass.schema().c_str());
		}
		analyzer::SegmenterOptions opts;
		if (!documentClass.schema().empty())
		{
			opts = m_textproc->getSegmenterOptions( documentClass.schema());
		}
		Reference<SegmenterInstanceInterface> segmenterinst( segmenter_->createInstance( opts));
		if (!segmenterinst.get()) throw std::runtime_error( _TXT("failed to create segmenter instance"));
		m_subsegmenterList.push_back( SubSegmenterDef( documentClass, segmenterinst.get(), selectexpr));
		segmenterinst.release();
	}
	CATCH_ERROR_MAP( _TXT("error in DocumentAnalyzerInstance::defineSubContentSegmenter: %s"), *m_errorhnd);
}

void DocumentAnalyzerInstance::addPatternLexem(
		const std::string& termtype,
		const std::string& selectexpr,
		TokenizerFunctionInstanceInterface* tokenizer,
		const std::vector<NormalizerFunctionInstanceInterface*>& normalizers,
		int priority)
{
	unsigned int featidx = m_featureConfigMap.defineFeature( FeatPatternLexem, termtype, selectexpr, tokenizer, normalizers, priority, analyzer::FeatureOptions());
	defineSelectorExpression( featidx, selectexpr);
}

void DocumentAnalyzerInstance::defineTokenPatternMatcher(
		const std::string& patternTypeName,
		PatternMatcherInstanceInterface* matcher,
		PatternTermFeederInstanceInterface* feeder)
{
	try
	{
		(void)m_postProcPatternMatchConfigMap.definePatternMatcher( patternTypeName, matcher, feeder, true);
	}
	CATCH_ERROR_MAP( _TXT("error defining token pattern match: %s"), *m_errorhnd);
}

void DocumentAnalyzerInstance::defineContentPatternMatcher(
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
	CATCH_ERROR_MAP( _TXT("error defining content pattern match: %s"), *m_errorhnd);
}

void DocumentAnalyzerInstance::addSearchIndexFeatureFromPatternMatch(
		const std::string& type,
		const std::string& patternTypeName,
		const std::vector<NormalizerFunctionInstanceInterface*>& normalizers,
		int priority,
		const analyzer::FeatureOptions& options)
{
	try
	{
		m_patternFeatureConfigMap.defineFeature( FeatSearchIndexTerm, type, patternTypeName, normalizers, priority, options);
	}
	CATCH_ERROR_MAP( _TXT("error defining search index feature from pattern matching result: %s"), *m_errorhnd);
}

void DocumentAnalyzerInstance::addForwardIndexFeatureFromPatternMatch(
		const std::string& type,
		const std::string& patternTypeName,
		const std::vector<NormalizerFunctionInstanceInterface*>& normalizers,
		int priority,
		const analyzer::FeatureOptions& options)
{
	try
	{
		m_patternFeatureConfigMap.defineFeature( FeatForwardIndexTerm, type, patternTypeName, normalizers, priority, options);
	}
	CATCH_ERROR_MAP( _TXT("error defining forward index feature from pattern matching result: %s"), *m_errorhnd);
}

void DocumentAnalyzerInstance::defineMetaDataFromPatternMatch(
		const std::string& metaname,
		const std::string& patternTypeName,
		const std::vector<NormalizerFunctionInstanceInterface*>& normalizers)
{
	try
	{
		m_patternFeatureConfigMap.defineFeature( FeatMetaData, metaname, patternTypeName, normalizers, 0/*priority*/, analyzer::FeatureOptions());
	}
	CATCH_ERROR_MAP( _TXT("error defining document meta data from pattern matching result: %s"), *m_errorhnd);
}

void DocumentAnalyzerInstance::defineAttributeFromPatternMatch(
		const std::string& attribname,
		const std::string& patternTypeName,
		const std::vector<NormalizerFunctionInstanceInterface*>& normalizers)
{
	try
	{
		m_patternFeatureConfigMap.defineFeature( FeatAttribute, attribname, patternTypeName, normalizers, 0/*priority*/, analyzer::FeatureOptions());
	}
	CATCH_ERROR_MAP( _TXT("error defining document attribute from pattern matching result: %s"), *m_errorhnd);
}

analyzer::Document DocumentAnalyzerInstance::analyze(
		const std::string& content,
		const analyzer::DocumentClass& dclass) const
{
	try
	{
		analyzer::Document rt;
		strus::local_ptr<DocumentAnalyzerContext>
			analyzerInstance( new DocumentAnalyzerContext( this, dclass, m_errorhnd));
		analyzerInstance->putInput( content.c_str(), content.size(), true);
		if (!analyzerInstance->analyzeNext( rt))
		{
			throw std::runtime_error( _TXT("analyzed content incomplete or empty"));
		}
		return rt;
	}
	CATCH_ERROR_MAP_RETURN( _TXT("error in document analyze: %s"), *m_errorhnd, analyzer::Document());
}

DocumentAnalyzerContextInterface* DocumentAnalyzerInstance::createContext( const analyzer::DocumentClass& dclass) const
{
	try
	{
		return new DocumentAnalyzerContext( this, dclass, m_errorhnd);
	}
	CATCH_ERROR_MAP_RETURN( _TXT("error in document analyzer create context: %s"), *m_errorhnd, 0);
}

static StructView getFeatureView( const FeatureConfig& cfg)
{
	typedef Reference<NormalizerFunctionInstanceInterface> NormalizerReference;
	StructView normalizerviews;
	std::vector<NormalizerReference>::const_iterator ni = cfg.normalizerlist().begin(), ne = cfg.normalizerlist().end();
	for (; ni != ne; ++ni)
	{
		normalizerviews( (*ni)->view());
	}
	return analyzer::FeatureView( cfg.name(), cfg.selectexpr(), cfg.tokenizer()->view(), normalizerviews, cfg.options(), cfg.priority());
}

StructView DocumentAnalyzerInstance::view() const
{
	try
	{
		StructView segmenterView( m_segmenter->view());
		StructView subcontents;
		{
			std::vector<SubSegmenterDef>::const_iterator si = m_subsegmenterList.begin(), se = m_subsegmenterList.end();
			for (; si != se; ++si)
			{
				subcontents( analyzer::SubContentDefinitionView( si->selectorPrefix, si->documentClass));
			}
		}
		StructView attributes;
		StructView metadata;
		StructView searchindex( StructView::Structure);
		StructView forwardindex( StructView::Structure);
		StructView searchfields;
		StructView searchstructures;
		StructView aggregators;
		StructView lexems;

		std::vector<FeatureConfig>::const_iterator fi = m_featureConfigMap.begin(), fe = m_featureConfigMap.end();
		for (; fi != fe; ++fi)
		{
			switch (fi->featureClass())
			{
				case FeatMetaData:
					metadata( getFeatureView( *fi));
					break;
				case FeatAttribute:
					attributes( getFeatureView( *fi));
					break;
				case FeatSearchIndexTerm:
					searchindex( getFeatureView( *fi));
					break;
				case FeatForwardIndexTerm:
					forwardindex( getFeatureView( *fi));
					break;
				case FeatPatternLexem:
					lexems( getFeatureView( *fi));
					break;
			}
		}
		std::vector<SeachIndexFieldConfig>::const_iterator
			li = m_fieldConfigList.begin(),le = m_fieldConfigList.end();
		for (;li != le; ++li)
		{
			searchfields( li->view());
		}
		std::vector<SeachIndexStructureConfig>::const_iterator
			xi = m_structureConfigList.begin(),xe = m_structureConfigList.end();
		for (;xi != xe; ++xi)
		{
			searchstructures( xi->view());
		}
		std::vector<StatisticsConfig>::const_iterator si = m_statistics.begin(), se = m_statistics.end();
		for (; si != se; ++si)
		{
			aggregators( analyzer::AggregatorView( si->name(), si->statfunc()->view()));
		}
		StructView subDocumentListView;
		std::vector<std::pair<std::string,std::string> >::const_iterator
			di = m_subDocumentList.begin(), de = m_subDocumentList.end();
		for (; di != de; ++di)
		{
			subDocumentListView( di->first, di->second);
		}
		return analyzer::DocumentAnalyzerView( 
			segmenterView, subcontents, subDocumentListView,
			attributes, metadata,
			searchindex, forwardindex,
			searchfields, searchstructures,
			aggregators, lexems);
	}
	CATCH_ERROR_MAP_RETURN( _TXT("error in document analyzer introspection: %s"), *m_errorhnd, StructView());
}


