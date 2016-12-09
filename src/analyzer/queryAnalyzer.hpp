/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef _STRUS_QUERY_ANALYZER_HPP_INCLUDED
#define _STRUS_QUERY_ANALYZER_HPP_INCLUDED
#include "strus/queryAnalyzerInterface.hpp"
#include "strus/normalizerFunctionInstanceInterface.hpp"
#include "strus/tokenizerFunctionInstanceInterface.hpp"
#include "strus/analyzer/token.hpp"
#include "featureConfigMap.hpp"
#include "patternFeatureConfigMap.hpp"
#include "patternMatchConfigMap.hpp"
#include <vector>
#include <string>
#include <utility>
#include <map>

namespace strus
{
/// \brief Forward declaration
class ErrorBufferInterface;

/// \brief Query analyzer implementation
class QueryAnalyzer
	:public QueryAnalyzerInterface
{
public:
	explicit QueryAnalyzer( ErrorBufferInterface* errorhnd)
		:m_featureConfigMap()
		,m_preProcPatternMatchConfigMap(),m_postProcPatternMatchConfigMap(),m_patternFeatureConfigMap()
		,m_fieldTypeFeatureMap(),m_errorhnd(errorhnd){}
	virtual ~QueryAnalyzer(){}

	virtual void addSearchIndexElement(
			const std::string& termtype,
			const std::string& fieldtype,
			TokenizerFunctionInstanceInterface* tokenizer,
			const std::vector<NormalizerFunctionInstanceInterface*>& normalizers);

	virtual void addMetaDataElement(
			const std::string& metaname,
			const std::string& fieldtype,
			TokenizerFunctionInstanceInterface* tokenizer,
			const std::vector<NormalizerFunctionInstanceInterface*>& normalizers);

	virtual void definePatternMatcherPostProc(
			const std::string& patternTypeName,
			PatternMatcherInstanceInterface* matcher,
			PatternTermFeederInstanceInterface* feeder);

	virtual void definePatternMatcherPreProc(
			const std::string& patternTypeName,
			PatternMatcherInstanceInterface* matcher,
			PatternLexerInstanceInterface* lexer,
			const std::vector<std::string>& fieldTypeNames);

	virtual void addSearchIndexFeatureFromPatternMatch(
			const std::string& type,
			const std::string& patternTypeName,
			const std::vector<NormalizerFunctionInstanceInterface*>& normalizers);

	virtual void defineMetaDataFromPatternMatch(
			const std::string& metaname,
			const std::string& patternTypeName,
			const std::vector<NormalizerFunctionInstanceInterface*>& normalizers);

	virtual QueryAnalyzerContextInterface* createContext() const;

public:/*QueryAnalyzerContext*/
	typedef std::pair<std::string,int> FieldTypeFeatureDef;
	typedef std::multimap<std::string,int> FieldTypeFeatureMap;

	const FeatureConfigMap& featureConfigMap() const				{return m_featureConfigMap;}
	const FieldTypeFeatureMap& fieldTypeFeatureMap() const				{return m_fieldTypeFeatureMap;}
	const PreProcPatternMatchConfigMap& preProcPatternMatchConfigMap() const	{return m_preProcPatternMatchConfigMap;}
	const PostProcPatternMatchConfigMap& postProcPatternMatchConfigMap() const	{return m_postProcPatternMatchConfigMap;}
	const PatternFeatureConfigMap& patternFeatureConfigMap() const			{return m_patternFeatureConfigMap;}

private:
	FeatureConfigMap m_featureConfigMap;
	PreProcPatternMatchConfigMap m_preProcPatternMatchConfigMap;
	PostProcPatternMatchConfigMap m_postProcPatternMatchConfigMap;
	PatternFeatureConfigMap m_patternFeatureConfigMap;
	FieldTypeFeatureMap m_fieldTypeFeatureMap;
	FieldTypeFeatureMap m_fieldTypePatternMap;
	ErrorBufferInterface* m_errorhnd;
};

}//namespace
#endif

