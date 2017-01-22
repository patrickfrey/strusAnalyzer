/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "featureConfig.hpp"
#include "private/internationalization.hpp"
#include "private/utils.hpp"
#include <vector>
#include <string>

using namespace strus;

const char* strus::featureClassName( FeatureClass i)
{
	static const char* ar[] = {"MetaData", "Attribute", "SearchIndexTerm", "ForwardIndexTerm", "PatternLexem"};
	return  ar[i];
}

FeatureConfig::FeatureConfig(
		const std::string& name_,
		TokenizerFunctionInstanceInterface* tokenizer_,
		const std::vector<NormalizerFunctionInstanceInterface*>& normalizers_,
		FeatureClass featureClass_,
		const analyzer::FeatureOptions& options_)
	:m_name(name_)
	,m_featureClass(featureClass_)
	,m_options(options_)
{
	if (tokenizer_->concatBeforeTokenize())
	{
		if (m_options.positionBind() != analyzer::BindContent)
		{
			throw strus::runtime_error( _TXT("illegal definition of a feature that has a tokenizer processing the content concatenated with positions bound to other features"));
		}
	}
	try
	{
		m_normalizerlist.reserve( normalizers_.size());
		std::vector<NormalizerFunctionInstanceInterface*>::const_iterator
			ci = normalizers_.begin(), ce = normalizers_.end();
		for (; ci != ce; ++ci)
		{
			m_normalizerlist.push_back( *ci);
		}
		m_tokenizer.reset( tokenizer_);
	}
	catch (const std::string& bad_alloc)
	{
		std::vector<NormalizerReference>::iterator
			ci = m_normalizerlist.begin(), ce = m_normalizerlist.end();
		for (; ci != ce; ++ci) ci->release();
		m_tokenizer.release();
		throw std::bad_alloc();
	}
}

std::string FeatureConfig::normalize( char const* tok, std::size_t toksize) const
{
	std::vector<NormalizerReference>::const_iterator
		ci = m_normalizerlist.begin(),
		ce = m_normalizerlist.end();
	if (ci == ce) return std::string( tok, toksize);

	std::string rt;
	std::string origstr;
	for (; ci != ce; ++ci)
	{
		rt = (*ci)->normalize( tok, toksize);
		if (ci + 1 != ce)
		{
			origstr.swap( rt);
			tok = origstr.c_str();
			toksize = origstr.size();
		}
	}
	return rt;
}

std::vector<analyzer::Token> FeatureConfig::tokenize( const char* src, std::size_t srcsize) const
{
	return m_tokenizer->tokenize( src, srcsize);
}


