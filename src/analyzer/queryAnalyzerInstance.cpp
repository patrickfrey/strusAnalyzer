/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "queryAnalyzerInstance.hpp"
#include "queryAnalyzerContext.hpp"
#include "strus/normalizerFunctionInstanceInterface.hpp"
#include "strus/tokenizerFunctionInstanceInterface.hpp"
#include "strus/errorBufferInterface.hpp"
#include "strus/base/string_conv.hpp"
#include "private/queryAnalyzerView.hpp"
#include "private/queryElementView.hpp"
#include "private/errorUtils.hpp"
#include "private/internationalization.hpp"
#include <stdexcept>
#include <set>
#include <map>

using namespace strus;

void QueryAnalyzerInstance::addElement(
		const std::string& termtype,
		const std::string& fieldtype,
		TokenizerFunctionInstanceInterface* tokenizer,
		const std::vector<NormalizerFunctionInstanceInterface*>& normalizers,
		int priority)
{
	try
	{
		unsigned int featidx = m_featureConfigMap.defineFeature( FeatSearchIndexTerm, termtype, fieldtype, tokenizer, normalizers, priority, analyzer::FeatureOptions());
		m_fieldTypeFeatureMap.insert( FieldTypeFeatureDef( fieldtype, featidx));
		m_searchIndexTermTypeSet.insert( string_conv::tolower( termtype));
	}
	CATCH_ERROR_MAP( _TXT("error adding feature: %s"), *m_errorhnd);
}

void QueryAnalyzerInstance::addPatternLexem(
		const std::string& termtype,
		const std::string& fieldtype,
		TokenizerFunctionInstanceInterface* tokenizer,
		const std::vector<NormalizerFunctionInstanceInterface*>& normalizers,
		int priority)
{
	try
	{
		unsigned int featidx = m_featureConfigMap.defineFeature( FeatPatternLexem, termtype, fieldtype, tokenizer, normalizers, priority, analyzer::FeatureOptions());
		m_fieldTypeFeatureMap.insert( FieldTypeFeatureDef( fieldtype, featidx));
	}
	CATCH_ERROR_MAP( _TXT("error adding meta data query element: %s"), *m_errorhnd);
}

void QueryAnalyzerInstance::defineTokenPatternMatcher(
		const std::string& patternTypeName,
		PatternMatcherInstanceInterface* matcher,
		PatternTermFeederInstanceInterface* feeder)
{
	try
	{
		(void)m_postProcPatternMatchConfigMap.definePatternMatcher( patternTypeName, matcher, feeder, false);
	}
	CATCH_ERROR_MAP( _TXT("error defining token pattern match: %s"), *m_errorhnd);
}

void QueryAnalyzerInstance::defineContentPatternMatcher(
		const std::string& patternTypeName,
		PatternMatcherInstanceInterface* matcher,
		PatternLexerInstanceInterface* lexer,
		const std::vector<std::string>& fieldTypeNames)
{
	try
	{
		unsigned int idx = m_preProcPatternMatchConfigMap.definePatternMatcher( patternTypeName, matcher, lexer, false);
		std::vector<std::string>::const_iterator
			si = fieldTypeNames.begin(), se = fieldTypeNames.end();
		for (; si != se; ++si)
		{
			m_fieldTypePatternMap.insert( FieldTypeFeatureDef( *si, idx));
		}
	}
	CATCH_ERROR_MAP( _TXT("error defining content pattern match: %s"), *m_errorhnd);
}

void QueryAnalyzerInstance::addElementFromPatternMatch(
		const std::string& type,
		const std::string& patternTypeName,
		const std::vector<NormalizerFunctionInstanceInterface*>& normalizers,
		int priority)
{
	try
	{
		m_patternFeatureConfigMap.defineFeature( FeatSearchIndexTerm, type, patternTypeName, normalizers, priority, analyzer::FeatureOptions());
	}
	CATCH_ERROR_MAP( _TXT("error in define index feature from pattern match: %s"), *m_errorhnd);
}

std::vector<std::string> QueryAnalyzerInstance::queryTermTypes() const
{
	try
	{
		std::vector<std::string> rt;
		std::vector<FeatureConfig>::const_iterator fi = m_featureConfigMap.begin(), fe = m_featureConfigMap.end();
		for (; fi != fe; ++fi)
		{
			rt.push_back( fi->name());
		}
		return rt;
	}
	CATCH_ERROR_MAP_RETURN( _TXT("error getting list of query term types: %s"), *m_errorhnd, std::vector<std::string>());
}

std::vector<std::string> QueryAnalyzerInstance::queryFieldTypes() const
{
	try
	{
		std::vector<std::string> rt;
		std::vector<FeatureConfig>::const_iterator fi = m_featureConfigMap.begin(), fe = m_featureConfigMap.end();
		for (; fi != fe; ++fi)
		{
			rt.push_back( fi->selectexpr());
		}
		return rt;
	}
	CATCH_ERROR_MAP_RETURN( _TXT("error getting list of query field types: %s"), *m_errorhnd, std::vector<std::string>());
}

QueryAnalyzerContextInterface* QueryAnalyzerInstance::createContext() const
{
	try
	{
		return new QueryAnalyzerContext( this, m_errorhnd);
	}
	CATCH_ERROR_MAP_RETURN( _TXT("error in QueryAnalyzerInstance::createContext: %s"), *m_errorhnd, 0);
}

static StructView getQueryElementView( const FeatureConfig& cfg)
{
	typedef Reference<NormalizerFunctionInstanceInterface> NormalizerReference;
	StructView normalizerviews;
	std::vector<NormalizerReference>::const_iterator ni = cfg.normalizerlist().begin(), ne = cfg.normalizerlist().end();
	for (; ni != ne; ++ni)
	{
		normalizerviews( (*ni)->view());
	}
	return analyzer::QueryElementView( cfg.name(), cfg.selectexpr(), cfg.tokenizer()->view(), normalizerviews, cfg.priority());
}

StructView QueryAnalyzerInstance::view() const
{
	try
	{
		StructView elements;
		StructView patternLexems;

		std::vector<FeatureConfig>::const_iterator fi = m_featureConfigMap.begin(), fe = m_featureConfigMap.end();
		for (; fi != fe; ++fi)
		{
			switch (fi->featureClass())
			{
				case FeatMetaData:
					throw strus::runtime_error(_TXT("internal: illegal %s feature definition in query"), "metadata");
					break;
				case FeatAttribute:
					throw strus::runtime_error(_TXT("internal: illegal %s feature definition in query"), "attribute");
					break;
				case FeatForwardIndexTerm:
					throw strus::runtime_error(_TXT("internal: illegal %s feature definition in query"), "forward index");
					break;
				case FeatSearchIndexTerm:
					elements( getQueryElementView( *fi));
					break;
				case FeatPatternLexem:
					patternLexems( getQueryElementView( *fi));
					break;
			}
		}
		return analyzer::QueryAnalyzerView( elements, patternLexems);
	}
	CATCH_ERROR_MAP_RETURN( _TXT("error in query analyzer create view: %s"), *m_errorhnd, StructView());
}

