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
class TextProcessorInterface;


/// \brief Query analyzer implementation
class QueryAnalyzer
	:public QueryAnalyzerInterface
{
public:
	explicit QueryAnalyzer(
			const TextProcessorInterface* textProcessor_);

	virtual ~QueryAnalyzer(){}

	virtual void definePhraseType(
			const std::string& phraseType,
			const std::string& featureType,
			TokenizerFunctionInstanceInterface* tokenizer,
			const std::vector<NormalizerFunctionInstanceInterface*>& normalizers);

	virtual std::vector<analyzer::Term> analyzePhrase(
			const std::string& phraseType,
			const std::string& content) const;

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
		typedef utils::SharedPtr<NormalizerExecutionContextInterface> NormalizerExecutionContextReference;
		typedef std::vector<NormalizerExecutionContextReference> NormalizerExecutionContextArray;
		typedef utils::SharedPtr<TokenizerExecutionContextInterface> TokenizerExecutionContextReference;

		FeatureContext( const FeatureConfig& config);
		FeatureContext( const FeatureContext& o)
			:m_config(o.m_config)
			,m_normalizerContextAr(o.m_normalizerContextAr)
			,m_tokenizerContext(o.m_tokenizerContext){}

		std::vector<analyzer::Token> tokenize( char const* segment, std::size_t segmentsize);
		std::string normalize( const char* tok, std::size_t toksize);

		const FeatureConfig* m_config;
		NormalizerExecutionContextArray m_normalizerContextAr;
		TokenizerExecutionContextReference m_tokenizerContext;
	};

	/// \brief Get the feature configuration for a named phrase type
	const FeatureConfig& featureConfig( const std::string& phraseType) const;

private:
	const TextProcessorInterface* m_textProcessor;
	std::map<std::string,FeatureConfig> m_featuremap;
};

}//namespace
#endif

