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
#include <vector>
#include <string>
#include <utility>
#include <map>
#include <boost/shared_ptr.hpp>

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

	virtual ~QueryAnalyzer();

	virtual void defineMethod(
			const std::string& method,
			const std::string& featureType,
			const TokenizerConfig& tokenizer,
			const NormalizerConfig& normalizer);

	virtual std::vector<analyzer::Term> analyzeChunk(
			const std::string& method,
			const std::string& content) const;

private:
	class FeatureConfig
	{
	public:
		FeatureConfig()
			:m_tokenizer(0),m_normalizer(0){}

		FeatureConfig( const std::string& featureType_,
				const TokenizerInterface* tokenizer_,
				const boost::shared_ptr<TokenizerInterface::Argument>& tokenizerarg_,
				const NormalizerInterface* normalizer_,
				const boost::shared_ptr<NormalizerInterface::Argument>& normalizerarg_)
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

		const std::string& featureType() const				{return m_featureType;}
		const TokenizerInterface* tokenizer() const			{return m_tokenizer;}
		const TokenizerInterface::Argument* tokenizerarg() const	{return m_tokenizerarg.get();}
		const NormalizerInterface* normalizer() const			{return m_normalizer;}
		const NormalizerInterface::Argument* normalizerarg() const	{return m_normalizerarg.get();}

	private:
		std::string m_featureType;
		const TokenizerInterface* m_tokenizer;
		boost::shared_ptr<TokenizerInterface::Argument> m_tokenizerarg;
		const NormalizerInterface* m_normalizer;
		boost::shared_ptr<NormalizerInterface::Argument> m_normalizerarg;
	};

	const FeatureConfig& featureConfig( const std::string& method) const;

private:
	const TextProcessorInterface* m_textProcessor;
	std::map<std::string,FeatureConfig> m_featuremap;
};

}//namespace
#endif

