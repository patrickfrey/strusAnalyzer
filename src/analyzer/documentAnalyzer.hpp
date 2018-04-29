/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef _STRUS_DOCUMENT_ANALYZER_HPP_INCLUDED
#define _STRUS_DOCUMENT_ANALYZER_HPP_INCLUDED
#include "strus/documentAnalyzerInterface.hpp"
#include "strus/normalizerFunctionInstanceInterface.hpp"
#include "strus/tokenizerFunctionInstanceInterface.hpp"
#include "strus/aggregatorFunctionInstanceInterface.hpp"
#include "strus/analyzer/documentClass.hpp"
#include "strus/analyzer/segmenterOptions.hpp"
#include "featureConfigMap.hpp"
#include "patternFeatureConfigMap.hpp"
#include "patternMatchConfigMap.hpp"
#include <vector>
#include <string>
#include <map>
#include <set>
#include <utility>
#include <stdexcept>

namespace strus
{
/// \brief Forward declaration
class ErrorBufferInterface;
/// \brief Forward declaration
class SegmenterInterface;
/// \brief Forward declaration
class SegmenterInstanceInterface;
/// \brief Forward declaration
class TextProcessorInterface;

/// \brief Document analyzer implementation
class DocumentAnalyzer
	:public DocumentAnalyzerInterface
{
public:
	DocumentAnalyzer(
			const TextProcessorInterface* textproc_,
			const SegmenterInterface* segmenter_,
			const analyzer::SegmenterOptions& opts,
			ErrorBufferInterface* errorhnd);

	virtual ~DocumentAnalyzer();

	virtual void addSearchIndexFeature(
			const std::string& type,
			const std::string& selectexpr,
			TokenizerFunctionInstanceInterface* tokenizer,
			const std::vector<NormalizerFunctionInstanceInterface*>& normalizers,
			const analyzer::FeatureOptions& options);

	virtual void addForwardIndexFeature(
			const std::string& type,
			const std::string& selectexpr,
			TokenizerFunctionInstanceInterface* tokenizer,
			const std::vector<NormalizerFunctionInstanceInterface*>& normalizers,
			const analyzer::FeatureOptions& options);

	virtual void defineMetaData(
			const std::string& fieldname,
			const std::string& selectexpr,
			TokenizerFunctionInstanceInterface* tokenizer,
			const std::vector<NormalizerFunctionInstanceInterface*>& normalizers);

	virtual void defineAttribute(
			const std::string& attribname,
			const std::string& selectexpr,
			TokenizerFunctionInstanceInterface* tokenizer,
			const std::vector<NormalizerFunctionInstanceInterface*>& normalizers);

	virtual void defineAggregatedMetaData(
			const std::string& fieldname,
			AggregatorFunctionInstanceInterface* statfunc);

	virtual void defineSubDocument(
			const std::string& subDocumentTypeName,
			const std::string& selectexpr);

	virtual void defineSubContent(
			const std::string& selectexpr,
			const analyzer::DocumentClass& documentClass);

	virtual void addPatternLexem(
			const std::string& termtype,
			const std::string& selectexpr,
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
			const std::vector<std::string>& selectexpr);

	virtual void addSearchIndexFeatureFromPatternMatch(
			const std::string& type,
			const std::string& patternTypeName,
			const std::vector<NormalizerFunctionInstanceInterface*>& normalizers,
			const analyzer::FeatureOptions& options);

	virtual void addForwardIndexFeatureFromPatternMatch(
			const std::string& type,
			const std::string& patternTypeName,
			const std::vector<NormalizerFunctionInstanceInterface*>& normalizers,
			const analyzer::FeatureOptions& options);

	virtual void defineMetaDataFromPatternMatch(
			const std::string& metaname,
			const std::string& patternTypeName,
			const std::vector<NormalizerFunctionInstanceInterface*>& normalizers);

	virtual void defineAttributeFromPatternMatch(
			const std::string& attribname,
			const std::string& patternTypeName,
			const std::vector<NormalizerFunctionInstanceInterface*>& normalizers);
	
	virtual analyzer::Document analyze(
			const std::string& content,
			const analyzer::DocumentClass& dclass) const;

	virtual DocumentAnalyzerContextInterface* createContext(
			const analyzer::DocumentClass& dclass) const;

	virtual analyzer::DocumentAnalyzerView view() const;

	virtual IntrospectionInterface* createIntrospection() const;

public:/*DocumentAnalyzerContext*/
	typedef Reference<AggregatorFunctionInstanceInterface> StatisticsReference;
	typedef std::set<std::string> TermTypeSet;

	class StatisticsConfig
	{
	public:
		StatisticsConfig(
				const std::string& name_,
				AggregatorFunctionInstanceInterface* statfunc_)
			:m_name(name_)
			,m_statfunc(statfunc_){}

		StatisticsConfig( const StatisticsConfig& o)
			:m_name(o.m_name)
			,m_statfunc(o.m_statfunc){}

		const std::string& name() const					{return m_name;}
		const StatisticsReference& statfunc() const			{return m_statfunc;}

	private:
		std::string m_name;
		StatisticsReference m_statfunc;
	};

	struct SubSegmenterDef
	{
		analyzer::DocumentClass documentClass;
		SegmenterInstanceInterface* segmenterInstance;
		std::string selectorPrefix;

		SubSegmenterDef( const analyzer::DocumentClass& documentClass_, SegmenterInstanceInterface* segmenterInstance_, const std::string& selectorPrefix_)
			:documentClass(documentClass_),segmenterInstance(segmenterInstance_),selectorPrefix(selectorPrefix_){}
		SubSegmenterDef( const SubSegmenterDef& o)
			:documentClass(o.documentClass),segmenterInstance(o.segmenterInstance),selectorPrefix(o.selectorPrefix){}
	};

	const SegmenterInstanceInterface* segmenter() const				{return m_segmenter;}
	const SubSegmenterDef* subsegmenter( unsigned int idx) const			{return (idx<m_subsegmenterList.size()) ? &m_subsegmenterList[idx]:(const SubSegmenterDef*)0;}
	const FeatureConfigMap& featureConfigMap() const				{return m_featureConfigMap;}
	const PreProcPatternMatchConfigMap& preProcPatternMatchConfigMap() const	{return m_preProcPatternMatchConfigMap;}
	const PostProcPatternMatchConfigMap& postProcPatternMatchConfigMap() const	{return m_postProcPatternMatchConfigMap;}
	const PatternFeatureConfigMap& patternFeatureConfigMap() const			{return m_patternFeatureConfigMap;}
	const TermTypeSet forwardIndexTermTypeSet() const				{return m_forwardIndexTermTypeSet;}
	const TermTypeSet searchIndexTermTypeSet() const				{return m_searchIndexTermTypeSet;}
	const std::vector<std::string>& subdoctypes() const				{return m_subdoctypear;}
	const std::vector<StatisticsConfig>& statisticsConfigs() const			{return m_statistics;}
	const std::vector<SubSegmenterDef>& subsegmenterList() const			{return m_subsegmenterList;}

private:
	void defineSelectorExpression( unsigned int featdidx, const std::string& selectexpr);
	void defineSubSection( int startId, int endId, const std::string& expression);

private:
	const TextProcessorInterface* m_textproc;
	SegmenterInstanceInterface* m_segmenter;
	std::vector<SubSegmenterDef> m_subsegmenterList;
	FeatureConfigMap m_featureConfigMap;
	PreProcPatternMatchConfigMap m_preProcPatternMatchConfigMap;
	PostProcPatternMatchConfigMap m_postProcPatternMatchConfigMap;
	PatternFeatureConfigMap m_patternFeatureConfigMap;
	std::vector<std::string> m_subdoctypear;
	std::vector<StatisticsConfig> m_statistics;
	TermTypeSet m_forwardIndexTermTypeSet;
	TermTypeSet m_searchIndexTermTypeSet;
	ErrorBufferInterface* m_errorhnd;
};

}//namespace
#endif

