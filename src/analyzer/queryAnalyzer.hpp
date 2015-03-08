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
#include "strus/normalizerInterface.hpp"
#include "strus/tokenizerInterface.hpp"
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
			const TokenizerConfig& tokenizer,
			const NormalizerConfig& normalizer);

	virtual std::vector<analyzer::Term> analyzePhrase(
			const std::string& phraseType,
			const std::string& content) const;

private:
	class FeatureConfig
	{
	public:
		FeatureConfig()
			:m_tokenizer(0),m_normalizer(0){}

		FeatureConfig( const std::string& featureType_,
				const TokenizerInterface* tokenizer_,
				const utils::SharedPtr<TokenizerInterface::Argument>& tokenizerarg_,
				const NormalizerInterface* normalizer_,
				const utils::SharedPtr<NormalizerInterface::Argument>& normalizerarg_)
			:m_featureType(featureType_)
			,m_tokenizer(tokenizer_)
			,m_tokenizerarg(tokenizerarg_)
			,m_normalizer(normalizer_)
			,m_normalizerarg(normalizerarg_){}

		FeatureConfig( const FeatureConfig& o)
			:m_featureType(o.m_featureType)
			,m_tokenizer(o.m_tokenizer)
			,m_tokenizerarg(o.m_tokenizerarg)
			,m_normalizer(o.m_normalizer)
			,m_normalizerarg(o.m_normalizerarg){}

		/// \brief Get the type of the features in the storage
		const std::string& featureType() const				{return m_featureType;}
		/// \brief Get the tokenizer function for tokenization of the phrase
		const TokenizerInterface* tokenizer() const			{return m_tokenizer;}
		/// \brief Get the tokenizer arguments for tokenization of the phrase
		const TokenizerInterface::Argument* tokenizerarg() const	{return m_tokenizerarg.get();}
		/// \brief Get the normalizer of the tokens for create the feature values
		const NormalizerInterface* normalizer() const			{return m_normalizer;}
		/// \brief Get the normalizer arguments
		const NormalizerInterface::Argument* normalizerarg() const	{return m_normalizerarg.get();}

	private:
		std::string m_featureType;
		const TokenizerInterface* m_tokenizer;
		utils::SharedPtr<TokenizerInterface::Argument> m_tokenizerarg;
		const NormalizerInterface* m_normalizer;
		utils::SharedPtr<NormalizerInterface::Argument> m_normalizerarg;
	};

	/// \brief Get the feature configuration for a named phrase type
	const FeatureConfig& featureConfig( const std::string& phraseType) const;

private:
	const TextProcessorInterface* m_textProcessor;
	std::map<std::string,FeatureConfig> m_featuremap;
};

}//namespace
#endif

