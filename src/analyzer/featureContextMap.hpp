/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef _STRUS_ANALYZER_FEATURE_CONTEXT_MAP_HPP_INCLUDED
#define _STRUS_ANALYZER_FEATURE_CONTEXT_MAP_HPP_INCLUDED
#include "strus/normalizerFunctionInstanceInterface.hpp"
#include "strus/tokenizerFunctionInstanceInterface.hpp"
#include "featureConfigMap.hpp"
#include "private/utils.hpp"
#include <vector>
#include <string>
#include <stdexcept>

namespace strus {

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

	std::string normalize( const char* tok, std::size_t toksize);

	const FeatureConfig* m_config;
	NormalizerFunctionContextArray m_normalizerContextAr;
	TokenizerFunctionContextReference m_tokenizerContext;
};

class FeatureContextMap
{
public:
	FeatureContextMap( const FeatureConfigMap& config);
	FeatureContextMap( const FeatureContextMap& o)
		:m_featureContextAr(o.m_featureContextAr){}
	~FeatureContextMap(){}

	FeatureContext& featureContext( int featidx)
	{
		if (featidx <= 0) throw std::logic_error("array bound write (document analyzer feature context)");
		return m_featureContextAr[ featidx-1];
	}
	const FeatureContext& featureContext( int featidx) const
	{
		if (featidx <= 0) throw std::logic_error("array bound read (document analyzer feature context)");
		return m_featureContextAr[ featidx-1];
	}

private:
	std::vector<FeatureContext> m_featureContextAr;
};

}//namespace
#endif

