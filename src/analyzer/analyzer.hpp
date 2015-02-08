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
#ifndef _STRUS_ANALYZER_HPP_INCLUDED
#define _STRUS_ANALYZER_HPP_INCLUDED
#include "strus/analyzerInterface.hpp"
#include "strus/reference.hpp"
#include <vector>
#include <string>
#include <utility>
#include <iostream>
#include <sstream>

namespace strus
{
/// \brief Forward declaration
class TokenMinerFactory;
/// \brief Forward declaration
class SegmenterInterface;
/// \brief Forward declaration
class NormalizerInterface;
/// \brief Forward declaration
class TokenizerInterface;

/// \brief Analyzer implementation based on textwolf
class Analyzer
	:public AnalyzerInterface
{
public:
	Analyzer(
			const TokenMinerFactory* tokenMinerFactory,
			SegmenterInterface* segmenter_);

	virtual ~Analyzer();

	virtual void defineSearchIndexFeature(
			const std::string& type,
			const std::string& selectexpr,
			const std::string& tokenizer,
			const std::string& normalizer)
	{
		defineFeature( FeatSearchIndexTerm, type, selectexpr, tokenizer, normalizer);
	}

	virtual void defineForwardIndexFeature(
			const std::string& type,
			const std::string& selectexpr,
			const std::string& tokenizer,
			const std::string& normalizer)
	{
		defineFeature( FeatForwardIndexTerm, type, selectexpr, tokenizer, normalizer);
	}

	virtual void defineMetaDataFeature(
			const std::string& fieldname,
			const std::string& selectexpr,
			const std::string& tokenizer,
			const std::string& normalizer)
	{
		defineFeature( FeatMetaData, type, selectexpr, tokenizer, normalizer);
	}

	virtual void defineAttributeFeature(
			const std::string& attribname,
			const std::string& selectexpr,
			const std::string& tokenizer,
			const std::string& normalizer)
	{
		defineFeature( FeatAttribute, type, selectexpr, tokenizer, normalizer);
	}


	virtual analyzer::Document analyzeDocument(
			const std::string& content) const;

	virtual std::vector<Term> analyzeTextChunk(
			const std::string& tokenizer,
			const std::string& normalizer,
			const std::string& content) const;

private:
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
				const NormalizerInterface* normalizer_,
				FeatureClass featureClass_)
			:m_name(name_)
			,m_tokenizer(tokenizer_)
			,m_normalizer(normalizer_)
			,m_featureClass(featureClass_){}
	
		const std::string& name() const			{return m_name;}
		const TokenizerInterface* tokenizer() const	{return m_tokenizer;}
		const NormalizerInterface* normalizer() const	{return m_normalizer;}
		FeatureClass featureClass() const		{return m_featureClass;}

		std::vector<tokenizer::Position>
			tokenize( const char* elem, std::size_t elemsize) const;

	private:
		std::string m_name;
		const TokenizerInterface* m_tokenizer;
		const NormalizerInterface* m_normalizer;
		FeatureClass m_featureClass;
	};

	void addExpression(
		const std::string& name,
		const std::string& expression,
		const TokenizerInterface* tokenizer,
		const NormalizerInterface* normalizer,
		FeatureClass featureClass);

	void defineFeature(
		FeatureClass featureClass,
		const std::string& name,
		const std::string& expression,
		const std::string& tokenizer,
		const std::string& normalizer);

	const FeatureConfig& featureConfig( int featidx) const;

private:
	const TokenMinerFactory* m_tokenMinerFactory;
	Reference<SegmenterInterface> m_segmenter;
	std::vector<FeatureConfig> m_featurear;
};

}//namespace
#endif

