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
#include "private/utils.hpp"
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
		:m_errorhnd(errorhnd){}
	virtual ~QueryAnalyzer(){}

	virtual void definePhraseType(
			const std::string& phraseType,
			const std::string& featureType,
			TokenizerFunctionInstanceInterface* tokenizer,
			const std::vector<NormalizerFunctionInstanceInterface*>& normalizers);

	virtual std::vector<analyzer::Term> analyzePhrase(
			const std::string& phraseType,
			const std::string& content) const;

	virtual std::vector<analyzer::TermVector> analyzePhraseBulk(
			const std::vector<Phrase>& phraseBulk) const;

private:
	class FeatureConfig
	{
	public:
		typedef utils::SharedPtr<TokenizerFunctionInstanceInterface> TokenizerReference;
		typedef utils::SharedPtr<NormalizerFunctionInstanceInterface> NormalizerReference;

		FeatureConfig()
			:m_tokenizer(0){}

		FeatureConfig( const std::string& featureType_,
				TokenizerFunctionInstanceInterface* tokenizer_,
				const std::vector<NormalizerFunctionInstanceInterface*>& normalizers_);

		FeatureConfig( const FeatureConfig& o)
			:m_featureType(o.m_featureType)
			,m_tokenizer(o.m_tokenizer)
			,m_normalizerlist(o.m_normalizerlist){}

		/// \brief Get the type of the features in the storage
		const std::string& featureType() const				{return m_featureType;}
		/// \brief Get the tokenizer function for tokenization of the phrase
		const TokenizerReference& tokenizer() const			{return m_tokenizer;}
		/// \brief Get the normalizer of the tokens for create the feature values
		const std::vector<NormalizerReference>& normalizerlist() const	{return m_normalizerlist;}

	private:
		std::string m_featureType;
		TokenizerReference m_tokenizer;
		std::vector<NormalizerReference> m_normalizerlist;
	};

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

		std::vector<analyzer::Token> tokenize( char const* segment, std::size_t segmentsize);
		std::string normalize( const char* tok, std::size_t toksize);

		const FeatureConfig* m_config;
		NormalizerFunctionContextArray m_normalizerContextAr;
		TokenizerFunctionContextReference m_tokenizerContext;
	};

	/// \brief Get the feature configuration for a named phrase type
	const FeatureConfig& featureConfig( const std::string& phraseType) const;

private:
	std::map<std::string,FeatureConfig> m_featuremap;
	ErrorBufferInterface* m_errorhnd;
};

}//namespace
#endif

