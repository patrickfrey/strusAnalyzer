/*
 * Copyright (c) 2016 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "patternFeatureConfig.hpp"
#include "featureConfig.hpp"
#include "private/internationalization.hpp"
#include "private/utils.hpp"
#include <vector>
#include <string>

using namespace strus;

PatternFeatureConfig::PatternFeatureConfig(
		const std::string& name_,
		const std::vector<NormalizerFunctionInstanceInterface*>& normalizers,
		FeatureClass featureClass_,
		const analyzer::FeatureOptions& options_)
	:m_name(name_)
	,m_featureClass(featureClass_)
	,m_options(options_)
{
	try
	{
		m_normalizerlist.reserve( normalizers.size());
		std::vector<NormalizerFunctionInstanceInterface*>::const_iterator
			ci = normalizers.begin(), ce = normalizers.end();
		for (; ci != ce; ++ci)
		{
			m_normalizerlist.push_back( *ci);
		}
	}
	catch (const std::string& bad_alloc)
	{
		std::vector<NormalizerReference>::iterator
			ci = m_normalizerlist.begin(), ce = m_normalizerlist.end();
		for (; ci != ce; ++ci) ci->release();
		throw std::bad_alloc();
	}
}

