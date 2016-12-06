/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "queryAnalyzer.hpp"
#include "queryAnalyzerContext.hpp"
#include "strus/normalizerFunctionContextInterface.hpp"
#include "strus/normalizerFunctionInstanceInterface.hpp"
#include "strus/tokenizerFunctionContextInterface.hpp"
#include "strus/tokenizerFunctionInstanceInterface.hpp"
#include "private/utils.hpp"
#include "private/errorUtils.hpp"
#include "private/internationalization.hpp"
#include <stdexcept>
#include <set>
#include <map>

using namespace strus;

void QueryAnalyzer::addSearchIndexElement(
		const std::string& termtype,
		const std::string& fieldtype,
		TokenizerFunctionInstanceInterface* tokenizer,
		const std::vector<NormalizerFunctionInstanceInterface*>& normalizers)
{
	try
	{
		unsigned int featidx = m_featureConfigMap.defineFeature( FeatSearchIndexTerm, termtype, tokenizer, normalizers, analyzer::FeatureOptions());
		m_fieldTypeFeatureMap.insert( std::pair<std::string,int>( fieldtype, featidx));
	}
	CATCH_ERROR_MAP( _TXT("error adding search index query feature: %s"), *m_errorhnd);
}

void QueryAnalyzer::addMetaDataElement(
		const std::string& metaname,
		const std::string& fieldtype,
		TokenizerFunctionInstanceInterface* tokenizer,
		const std::vector<NormalizerFunctionInstanceInterface*>& normalizers)
{
	try
	{
		unsigned int featidx = m_featureConfigMap.defineFeature( FeatMetaData, metaname, tokenizer, normalizers, analyzer::FeatureOptions());
		m_fieldTypeFeatureMap.insert( std::pair<std::string,int>( fieldtype, featidx));
	}
	CATCH_ERROR_MAP( _TXT("error adding meta data query element: %s"), *m_errorhnd);
}

void QueryAnalyzer::definePatternMatcherPostProc(
		const std::string& patternTypeName,
		PatternMatcherInstanceInterface* matcher,
		PatternTermFeederInstanceInterface* feeder)
{
	try
	{
	}
	CATCH_ERROR_MAP( _TXT("error defining post processing pattern match: %s"), *m_errorhnd);
}

void QueryAnalyzer::definePatternMatcherPreProc(
		const std::string& patternTypeName,
		PatternMatcherInstanceInterface* matcher,
		PatternLexerInstanceInterface* lexer,
		const std::vector<std::string>& selectexpr)
{
	try
	{
	}
	CATCH_ERROR_MAP( _TXT("error defining pre processing pattern match: %s"), *m_errorhnd);
}

void QueryAnalyzer::addSearchIndexFeatureFromPatternMatch(
		const std::string& type,
		const std::string& patternTypeName,
		const std::vector<NormalizerFunctionInstanceInterface*>& normalizers)
{
	try
	{
	}
	CATCH_ERROR_MAP( _TXT("error in define index feature from pattern match: %s"), *m_errorhnd);
}

void QueryAnalyzer::defineMetaDataFromPatternMatch(
		const std::string& metaname,
		const std::string& patternTypeName,
		const std::vector<NormalizerFunctionInstanceInterface*>& normalizers)
{
	try
	{
	}
	CATCH_ERROR_MAP( _TXT("error in define meta data from pattern match: %s"), *m_errorhnd);
}

QueryAnalyzerContextInterface* QueryAnalyzer::createContext() const
{
	try
	{
		return new QueryAnalyzerContext( this, m_errorhnd);
	}
	CATCH_ERROR_MAP_RETURN( _TXT("error in QueryAnalyzer::createContext: %s"), *m_errorhnd, 0);
}


