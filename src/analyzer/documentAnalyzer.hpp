/*
---------------------------------------------------------------------
    The C++ library strus implements basic operations to build
    a search engine for structured search on unstructured data.

    Copyright (C) 2013,2014 Patrick Frey

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

--------------------------------------------------------------------

	The latest version of strus can be found at 'http://github.com/patrickfrey/strus'
	For documentation see 'http://patrickfrey.github.com/strus'

--------------------------------------------------------------------
*/
#ifndef _STRUS_DOCUMENT_ANALYZER_HPP_INCLUDED
#define _STRUS_DOCUMENT_ANALYZER_HPP_INCLUDED
#include "strus/documentAnalyzerInterface.hpp"
#include "strus/documentAnalyzerInstanceInterface.hpp"
#include "strus/segmenterInterface.hpp"
#include "strus/segmenterInstanceInterface.hpp"
#include "strus/normalizerInterface.hpp"
#include "strus/tokenizerInterface.hpp"
#include "private/utils.hpp"
#include <vector>
#include <string>
#include <map>
#include <utility>

namespace strus
{
/// \brief Forward declaration
class TextProcessorInterface;

/// \brief Document analyzer implementation
class DocumentAnalyzer
	:public DocumentAnalyzerInterface
{
public:
	DocumentAnalyzer(
			const TextProcessorInterface* textProcessor_,
			SegmenterInterface* segmenter_);

	virtual ~DocumentAnalyzer()
	{
		delete m_segmenter;
	}

	virtual void addSearchIndexFeature(
			const std::string& type,
			const std::string& selectexpr,
			const TokenizerConfig& tokenizer,
			const NormalizerConfig& normalizer,
			const FeatureOptions& options)
	{
		defineFeature( FeatSearchIndexTerm, type, selectexpr, tokenizer, normalizer, options);
	}

	virtual void addForwardIndexFeature(
			const std::string& type,
			const std::string& selectexpr,
			const TokenizerConfig& tokenizer,
			const NormalizerConfig& normalizer,
			const FeatureOptions& options)
	{
		defineFeature( FeatForwardIndexTerm, type, selectexpr, tokenizer, normalizer, options);
	}

	virtual void defineMetaData(
			const std::string& fieldname,
			const std::string& selectexpr,
			const TokenizerConfig& tokenizer,
			const NormalizerConfig& normalizer)
	{
		defineFeature( FeatMetaData, fieldname, selectexpr, tokenizer, normalizer, FeatureOptions());
	}

	virtual void defineAttribute(
			const std::string& attribname,
			const std::string& selectexpr,
			const TokenizerConfig& tokenizer,
			const NormalizerConfig& normalizer)
	{
		defineFeature( FeatAttribute, attribname, selectexpr, tokenizer, normalizer, FeatureOptions());
	}

	virtual void defineSubDocument(
			const std::string& subDocumentTypeName,
			const std::string& selectexpr);

	virtual analyzer::Document analyze( const std::string& content) const;

	virtual analyzer::Document analyze( std::istream& input) const;

	virtual DocumentAnalyzerInstanceInterface* createDocumentAnalyzerInstance( std::istream& input) const;

public:
	enum FeatureClass
	{
		FeatMetaData,
		FeatAttribute,
		FeatSearchIndexTerm,
		FeatForwardIndexTerm
	};
	static const char* featureClassName( FeatureClass i)
	{
		static const char* ar[] = {"MetaData", "Attribute", "SearchIndexTerm", "ForwardIndexTerm"};
		return  ar[i];
	}

	class FeatureConfig
	{
	public:
		FeatureConfig( const std::string& name_,
				const TokenizerInterface* tokenizer_,
				const utils::SharedPtr<TokenizerInterface::Argument>& tokenizerarg_,
				const NormalizerInterface* normalizer_,
				const utils::SharedPtr<NormalizerInterface::Argument>& normalizerarg_,
				FeatureClass featureClass_,
				const FeatureOptions& options_)
			:m_name(name_)
			,m_tokenizer(tokenizer_)
			,m_tokenizerarg(tokenizerarg_)
			,m_normalizer(normalizer_)
			,m_normalizerarg(normalizerarg_)
			,m_featureClass(featureClass_)
			,m_options(options_){}

		FeatureConfig( const FeatureConfig& o)
			:m_name(o.m_name)
			,m_tokenizer(o.m_tokenizer)
			,m_tokenizerarg(o.m_tokenizerarg)
			,m_normalizer(o.m_normalizer)
			,m_normalizerarg(o.m_normalizerarg)
			,m_featureClass(o.m_featureClass)
			,m_options(o.m_options){}
	
		const std::string& name() const					{return m_name;}
		const TokenizerInterface* tokenizer() const			{return m_tokenizer;}
		const TokenizerInterface::Argument* tokenizerarg() const	{return m_tokenizerarg.get();}
		const NormalizerInterface* normalizer() const			{return m_normalizer;}
		const NormalizerInterface::Argument* normalizerarg() const	{return m_normalizerarg.get();}
		FeatureClass featureClass() const				{return m_featureClass;}
		FeatureOptions options() const					{return m_options;}

	private:
		std::string m_name;
		const TokenizerInterface* m_tokenizer;
		utils::SharedPtr<TokenizerInterface::Argument> m_tokenizerarg;
		const NormalizerInterface* m_normalizer;
		utils::SharedPtr<NormalizerInterface::Argument> m_normalizerarg;
		FeatureClass m_featureClass;
		FeatureOptions m_options;
	};

private:
	void defineFeature(
		FeatureClass featureClass,
		const std::string& name,
		const std::string& expression,
		const TokenizerConfig& tokenizer,
		const NormalizerConfig& normalizer,
		const FeatureOptions& options);

	const FeatureConfig& featureConfig( int featidx) const;

private:
	friend class DocumentAnalyzerInstance;
	const TextProcessorInterface* m_textProcessor;
	SegmenterInterface* m_segmenter;
	std::vector<FeatureConfig> m_featurear;
	std::vector<std::string> m_subdoctypear;
};


class ParserContext
{
public:
	ParserContext( const std::vector<DocumentAnalyzer::FeatureConfig>& config);
	~ParserContext()
	{
		cleanup();
	}

	void cleanup();

	NormalizerInterface::Context* normalizerContext( int featidx) const;
	TokenizerInterface::Context* tokenizerContext( int featidx) const;

private:
	TokenizerInterface::Context** m_tokenizerContextAr;
	NormalizerInterface::Context** m_normalizerContextAr;
	std::size_t m_size;
};


class DocumentAnalyzerInstance
	:public DocumentAnalyzerInstanceInterface
{
public:
	explicit DocumentAnalyzerInstance( const DocumentAnalyzer* analyzer_, std::istream& input_)
		:m_analyzer(analyzer_)
		,m_segmenter(m_analyzer->m_segmenter->createInstance( input_))
		,m_parserContext(analyzer_->m_featurear)
	{
		m_subdocstack.push_back( analyzer::Document());
	}

	virtual ~DocumentAnalyzerInstance()
	{
		delete m_segmenter;
	}

	virtual analyzer::Document analyzeNext();

	virtual bool hasMore() const
	{
		return !m_subdocstack.empty();
	}

private:
	void clearTermMaps();
	void mapPositions( analyzer::Document& res) const;
	void processDocumentSegment( analyzer::Document& res, int featidx, std::size_t rel_position, const char* elem, std::size_t elemsize);
	void concatDocumentSegment( int featidx, std::size_t rel_position, const char* elem, std::size_t elemsize);
	void processConcatenated( analyzer::Document& res);

private:
	const DocumentAnalyzer* m_analyzer;
	SegmenterInstanceInterface* m_segmenter;
	ParserContext m_parserContext;
	std::vector<analyzer::Document> m_subdocstack;

	struct Chunk
	{
		Chunk()
			:position(0){}
		Chunk( std::size_t position_, const std::string& content_)
			:position(position_),content(content_){}
		Chunk( const Chunk& o)
			:position(o.position),content(o.content){}
	
		std::size_t position;
		std::string content;
	};
	
	typedef std::map<int,Chunk> ConcatenatedMap;

	ConcatenatedMap m_concatenatedMap;

	std::vector<analyzer::Term> m_searchTerms;
	std::vector<analyzer::Term> m_forwardTerms;
};

}//namespace
#endif

