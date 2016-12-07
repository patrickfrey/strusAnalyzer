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
	static const char* ar[] = {"MetaData", "Attribute", "SearchIndexTerm", "ForwardIndexTerm"};
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
	// PF:NOTE: The following order of code ensures that if this constructor fails then no tokenizer or normalizer is copied, because otherwise they will be free()d twice:
	m_normalizerlist.reserve( normalizers_.size());
	std::vector<NormalizerFunctionInstanceInterface*>::const_iterator
		ci = normalizers_.begin(), ce = normalizers_.end();
	for (; ci != ce; ++ci)
	{
		m_normalizerlist.push_back( *ci);
	}
	m_tokenizer.reset( tokenizer_);
}

