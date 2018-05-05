/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "queryAnalyzer.hpp"
#include "queryAnalyzerContext.hpp"
#include "strus/normalizerFunctionInstanceInterface.hpp"
#include "strus/tokenizerFunctionInstanceInterface.hpp"
#include "strus/errorBufferInterface.hpp"
#include "strus/base/string_conv.hpp"
#include "private/errorUtils.hpp"
#include "private/internationalization.hpp"
#include <stdexcept>
#include <set>
#include <map>

using namespace strus;

void QueryAnalyzer::addElement(
		const std::string& termtype,
		const std::string& fieldtype,
		TokenizerFunctionInstanceInterface* tokenizer,
		const std::vector<NormalizerFunctionInstanceInterface*>& normalizers)
{
	try
	{
		unsigned int featidx = m_featureConfigMap.defineFeature( FeatSearchIndexTerm, termtype, fieldtype, tokenizer, normalizers, analyzer::FeatureOptions());
		m_fieldTypeFeatureMap.insert( FieldTypeFeatureDef( fieldtype, featidx));
		m_searchIndexTermTypeSet.insert( string_conv::tolower( termtype));
	}
	CATCH_ERROR_MAP( _TXT("error adding feature: %s"), *m_errorhnd);
}

void QueryAnalyzer::addPatternLexem(
		const std::string& termtype,
		const std::string& fieldtype,
		TokenizerFunctionInstanceInterface* tokenizer,
		const std::vector<NormalizerFunctionInstanceInterface*>& normalizers)
{
	try
	{
		unsigned int featidx = m_featureConfigMap.defineFeature( FeatPatternLexem, termtype, fieldtype, tokenizer, normalizers, analyzer::FeatureOptions());
		m_fieldTypeFeatureMap.insert( FieldTypeFeatureDef( fieldtype, featidx));
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
		(void)m_postProcPatternMatchConfigMap.definePatternMatcher( patternTypeName, matcher, feeder, false);
	}
	CATCH_ERROR_MAP( _TXT("error defining post processing pattern match: %s"), *m_errorhnd);
}

void QueryAnalyzer::definePatternMatcherPreProc(
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
	CATCH_ERROR_MAP( _TXT("error defining pre processing pattern match: %s"), *m_errorhnd);
}

void QueryAnalyzer::addElementFromPatternMatch(
		const std::string& type,
		const std::string& patternTypeName,
		const std::vector<NormalizerFunctionInstanceInterface*>& normalizers)
{
	try
	{
		m_patternFeatureConfigMap.defineFeature( FeatSearchIndexTerm, type, patternTypeName, normalizers, analyzer::FeatureOptions());
	}
	CATCH_ERROR_MAP( _TXT("error in define index feature from pattern match: %s"), *m_errorhnd);
}

QueryAnalyzerContextInterface* QueryAnalyzer::createContext() const
{
	try
	{
		return new QueryAnalyzerContext( this, m_errorhnd);
	}
	CATCH_ERROR_MAP_RETURN( _TXT("error in QueryAnalyzer::createContext: %s"), *m_errorhnd, 0);
}


