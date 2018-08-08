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
#include "strus/analyzer/featureOptions.hpp"
#include "strus/reference.hpp"
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
	FeatForwardIndexTerm,
	FeatPatternLexem
};

const char* featureClassName( FeatureClass i);

class FeatureConfig
{
public:
	FeatureConfig( const std::string& name_,
			const std::string& selectexpr_,
			TokenizerFunctionInstanceInterface* tokenizer,
			const std::vector<NormalizerFunctionInstanceInterface*>& normalizers,
			int priority_,
			FeatureClass featureClass_,
			const analyzer::FeatureOptions& options_);

	FeatureConfig( const FeatureConfig& o)
		:m_name(o.m_name)
		,m_selectexpr(o.m_selectexpr)
		,m_tokenizer(o.m_tokenizer)
		,m_normalizerlist(o.m_normalizerlist)
		,m_priority(o.m_priority)
		,m_featureClass(o.m_featureClass)
		,m_options(o.m_options){}

	typedef Reference<NormalizerFunctionInstanceInterface> NormalizerReference;
	typedef Reference<TokenizerFunctionInstanceInterface> TokenizerReference;

	const std::string& name() const					{return m_name;}
	const std::string& selectexpr() const				{return m_selectexpr;}
	const TokenizerReference& tokenizer() const			{return m_tokenizer;}
	const std::vector<NormalizerReference>& normalizerlist() const	{return m_normalizerlist;}
	int priority() const						{return m_priority;}
	FeatureClass featureClass() const				{return m_featureClass;}
	const analyzer::FeatureOptions& options() const			{return m_options;}

	std::string normalize( char const* tok, std::size_t toksize) const;
	std::vector<analyzer::Token> tokenize( const char* src, std::size_t srcsize) const;

private:
	std::string m_name;
	std::string m_selectexpr;
	TokenizerReference m_tokenizer;
	std::vector<NormalizerReference> m_normalizerlist;
	int m_priority;
	FeatureClass m_featureClass;
	analyzer::FeatureOptions m_options;
};

} //namespace
#endif

