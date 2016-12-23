/*
* Copyright (c) 2016 Patrick P. Frey
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
#include "patternFeatureContextMap.hpp"
#include "strus/normalizerFunctionInstanceInterface.hpp"
#include <map>
#include <vector>
#include <string>

#error DEPREACATED
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
	if (!m_map) return 0;
	std::map<std::string,std::size_t>::const_iterator mi = m_map->find( patternTypeName);
	if (mi == m_map->end()) return 0;
	return &m_ar[ mi->second];
}


