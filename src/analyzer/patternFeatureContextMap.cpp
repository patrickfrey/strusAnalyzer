/*
* Copyright (c) 2016 Patrick P. Frey
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
#include "patternFeatureContextMap.hpp"
#include "strus/normalizerFunctionContextInterface.hpp"
#include <map>
#include <vector>
#include <string>

using namespace strus;

PatternFeatureContext::PatternFeatureContext( const PatternFeatureConfig* config_)
	:m_config(config_)
{
	m_normalizerlist.reserve( m_config->normalizerlist().size());
	std::vector<Reference<NormalizerFunctionInstanceInterface> >::const_iterator
		ni = m_config->normalizerlist().begin(), ne = m_config->normalizerlist().end();
	for (; ni != ne; ++ni)
	{
		m_normalizerlist.push_back( (*ni)->createFunctionContext());
	}
}

std::string PatternFeatureContext::normalize( const std::string& value)
{
	std::vector<NormalizerReference>::iterator
		ci = m_normalizerlist.begin(),
		ce = m_normalizerlist.end();

	if (ci == ce) return value;

	const char* tok = value.c_str();
	std::size_t toksize = value.size();
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

PatternFeatureContextMap::PatternFeatureContextMap( const PatternFeatureConfigMap& configMap)
	:m_ar(),m_map(&configMap.m_map)
{
	m_ar.reserve( configMap.m_ar.size());
	std::vector<PatternFeatureConfig>::const_iterator
		ci = configMap.m_ar.begin(), ce = configMap.m_ar.end();
	for (; ci != ce; ++ci)
	{
		m_ar.push_back( PatternFeatureContext( &*ci));
	}
}

PatternFeatureContext* PatternFeatureContextMap::getContext( const std::string& patternTypeName)
{
	std::map<std::string,std::size_t>::const_iterator mi = m_map->find( patternTypeName);
	if (mi == m_map->end()) return 0;
	return &m_ar[ mi->second];
}


