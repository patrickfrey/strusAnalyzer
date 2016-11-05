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
#include "strus/documentAnalyzerContextInterface.hpp"
#include "strus/segmenterInterface.hpp"
#include "strus/segmenterInstanceInterface.hpp"
#include "strus/segmenterContextInterface.hpp"
#include "strus/analyzer/documentClass.hpp"
#include "strus/normalizerFunctionInstanceInterface.hpp"
#include "strus/tokenizerFunctionInstanceInterface.hpp"
#include "strus/aggregatorFunctionInstanceInterface.hpp"
#include "strus/analyzer/token.hpp"
#include "private/utils.hpp"
#include "featureConfig.hpp"
#include <vector>
#include <string>
#include <map>
#include <utility>
#include <stdexcept>

namespace strus
{
/// \brief Forward declaration
class ErrorBufferInterface;

/// \brief Document analyzer implementation
class DocumentAnalyzer
	:public DocumentAnalyzerInterface
{
public:
	DocumentAnalyzer( const SegmenterInterface* segmenter_, const analyzer::SegmenterOptions& opts, ErrorBufferInterface* errorhnd);

	virtual ~DocumentAnalyzer()
	{
		delete m_segmenter;
	}

	virtual void addSearchIndexFeature(
			const std::string& type,
			const std::string& selectexpr,
			TokenizerFunctionInstanceInterface* tokenizer,
			const std::vector<NormalizerFunctionInstanceInterface*>& normalizers,
			const FeatureOptions& options);

	virtual void addForwardIndexFeature(
			const std::string& type,
			const std::string& selectexpr,
			TokenizerFunctionInstanceInterface* tokenizer,
			const std::vector<NormalizerFunctionInstanceInterface*>& normalizers,
			const FeatureOptions& options);

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

	virtual analyzer::Document analyze(
			const std::string& content,
			const analyzer::DocumentClass& dclass) const;

	virtual DocumentAnalyzerContextInterface* createContext(
			const analyzer::DocumentClass& dclass) const;

public:
	typedef utils::SharedPtr<AggregatorFunctionInstanceInterface> StatisticsReference;
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

private:
	void defineFeature(
		FeatureClass featureClass,
		const std::string& name,
		const std::string& expression,
		TokenizerFunctionInstanceInterface* tokenizer,
		const std::vector<NormalizerFunctionInstanceInterface*>& normalizers,
		const FeatureOptions& options);

	const FeatureConfig& featureConfig( int featidx) const;

private:
	friend class DocumentAnalyzerContext;
	SegmenterInstanceInterface* m_segmenter;
	std::vector<FeatureConfig> m_featurear;
	std::vector<std::string> m_subdoctypear;
	std::vector<StatisticsConfig> m_statistics;
	ErrorBufferInterface* m_errorhnd;
};


class ParserContext
{
public:
	ParserContext( const std::vector<FeatureConfig>& config);
	ParserContext( const ParserContext& o)
		:m_featureContextAr(o.m_featureContextAr){}
	~ParserContext(){}

	struct FeatureContext
	{
		typedef utils::SharedPtr<NormalizerFunctionContextInterface> NormalizerFunctionContextReference;
		typedef std::vector<NormalizerFunctionContextReference> NormalizerFunctionContextArray;
		typedef utils::SharedPtr<TokenizerFunctionContextInterface> TokenizerFunctionContextReference;

		FeatureContext( const FeatureConfig& config);
		FeatureContext( const FeatureContext& o)
			:m_config(o.m_config)
			,m_normalizerContextAr(o.m_normalizerContextAr)
			,m_tokenizerContext(o.m_tokenizerContext){}

		std::string normalize( const char* tok, std::size_t toksize);

		const FeatureConfig* m_config;
		NormalizerFunctionContextArray m_normalizerContextAr;
		TokenizerFunctionContextReference m_tokenizerContext;
	};

	FeatureContext& featureContext( int featidx)
	{
		if (featidx <= 0) throw std::logic_error("array bound write (document analyzer parser context)");
		return m_featureContextAr[ featidx-1];
	}

private:
	std::vector<FeatureContext> m_featureContextAr;
};


class BindTerm
	:public analyzer::Term
{
public:
	BindTerm( const BindTerm& o)
		:analyzer::Term(o),m_posbind(o.m_posbind){}
	BindTerm( const std::string& t, const std::string& v, unsigned int p, analyzer::PositionBind b)
		:analyzer::Term(t,v,p),m_posbind(b){}

	analyzer::PositionBind posbind() const		{return m_posbind;}
private:
	analyzer::PositionBind m_posbind;
};


class DocumentAnalyzerContext
	:public DocumentAnalyzerContextInterface
{
public:
	DocumentAnalyzerContext( const DocumentAnalyzer* analyzer_, const analyzer::DocumentClass& dclass, ErrorBufferInterface* errorhnd);

	virtual ~DocumentAnalyzerContext()
	{
		delete m_segmenter;
	}

	virtual void putInput(const char* chunk, std::size_t chunksize, bool eof);

	virtual bool analyzeNext( analyzer::Document& doc);

private:
	struct SegPosDef
	{
		std::size_t start_strpos;
		std::size_t end_strpos;
		std::size_t segpos;

		SegPosDef( std::size_t start_strpos_, std::size_t end_strpos_, std::size_t segpos_)
			:start_strpos(start_strpos_),end_strpos(end_strpos_),segpos(segpos_){}
		SegPosDef( const SegPosDef& o)
			:start_strpos(o.start_strpos),end_strpos(o.end_strpos),segpos(o.segpos){}
	};

	void clearTermMaps();
	void mapPositions( analyzer::Document& res) const;
	void mapStatistics( analyzer::Document& res) const;
	///\param[in] samePosition true, if all elements get the same position (bind predecessor, bind successor)
	void processDocumentSegment( analyzer::Document& res, int featidx, std::size_t rel_position, const char* elem, std::size_t elemsize, const std::vector<SegPosDef>& concatposmap);
	void processContentTokens( std::vector<BindTerm>& result, ParserContext::FeatureContext& feat, const std::vector<analyzer::Token>& tokens, const char* segsrc, std::size_t rel_position, const std::vector<SegPosDef>& concatposmap) const;
	void concatDocumentSegment( int featidx, std::size_t rel_position, const char* elem, std::size_t elemsize);
	void processConcatenated( analyzer::Document& res);

private:
	struct Chunk
	{
		Chunk()
			:position(0){}
		Chunk( std::size_t position_, const std::string& content_)
			:position(position_),content(content_),concatposmap(){}
		Chunk( std::size_t position_, const std::string& content_, std::size_t segpos)
			:position(position_),content(content_),concatposmap()
		{
			concatposmap.push_back( SegPosDef( 0, content.size(), segpos));
		}
		Chunk( const Chunk& o)
			:position(o.position),content(o.content),concatposmap(o.concatposmap){}
	
		std::size_t position;
		std::string content;
		std::vector<SegPosDef> concatposmap;
	};
	
	typedef std::map<int,Chunk> ConcatenatedMap;

private:
	const DocumentAnalyzer* m_analyzer;
	SegmenterContextInterface* m_segmenter;
	ParserContext m_parserContext;
	std::vector<analyzer::Document> m_subdocstack;

	ConcatenatedMap m_concatenatedMap;

	std::vector<BindTerm> m_searchTerms;
	std::vector<BindTerm> m_forwardTerms;
	bool m_eof;
	SegmenterPosition m_curr_position;
	SegmenterPosition m_start_position;
	ErrorBufferInterface* m_errorhnd;
};

}//namespace
#endif

