/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef _STRUS_ANALYZER_FEATURE_CONFIG_HPP_INCLUDED
#define _STRUS_ANALYZER_FEATURE_CONFIG_HPP_INCLUDED
#include "strus/normalizerFunctionInstanceInterface.hpp"
#include "strus/tokenizerFunctionInstanceInterface.hpp"
#include "strus/aggregatorFunctionInstanceInterface.hpp"
#include "strus/documentAnalyzerInterface.hpp"
#include "strus/analyzer/token.hpp"
#include "strus/analyzer/featureOptions.hpp"
#include "private/utils.hpp"
#include <vector>
#include <string>

namespace strus
{
/// \brief Forward declaration
class ErrorBufferInterface;

enum FeatureClass
{
	FeatMetaData,
	FeatAttribute,
	FeatSearchIndexTerm,
	FeatForwardIndexTerm
};

const char* featureClassName( FeatureClass i);

class FeatureConfig
{
public:
	FeatureConfig( const std::string& name_,
			TokenizerFunctionInstanceInterface* tokenizer,
			const std::vector<NormalizerFunctionInstanceInterface*>& normalizers,
			FeatureClass featureClass_,
			const analyzer::FeatureOptions& options_);

	FeatureConfig( const FeatureConfig& o)
		:m_name(o.m_name)
		,m_tokenizer(o.m_tokenizer)
		,m_normalizerlist(o.m_normalizerlist)
		,m_featureClass(o.m_featureClass)
		,m_options(o.m_options){}

	typedef utils::SharedPtr<NormalizerFunctionInstanceInterface> NormalizerReference;
	typedef utils::SharedPtr<TokenizerFunctionInstanceInterface> TokenizerReference;

	const std::string& name() const					{return m_name;}
	const TokenizerReference& tokenizer() const			{return m_tokenizer;}
	const std::vector<NormalizerReference>& normalizerlist() const	{return m_normalizerlist;}
	FeatureClass featureClass() const				{return m_featureClass;}
	analyzer::FeatureOptions options() const			{return m_options;}

private:
	std::string m_name;
	TokenizerReference m_tokenizer;
	std::vector<NormalizerReference> m_normalizerlist;
	FeatureClass m_featureClass;
	analyzer::FeatureOptions m_options;
};

} //namespace
#endif

