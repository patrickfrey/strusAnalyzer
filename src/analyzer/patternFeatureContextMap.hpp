/*
 * Copyright (c) 2016 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef _STRUS_DOCUMENT_ANALYZER_PATTERN_FEATURE_CONTEXT_MAP_HPP_INCLUDED
#define _STRUS_DOCUMENT_ANALYZER_PATTERN_FEATURE_CONTEXT_MAP_HPP_INCLUDED
#include "patternFeatureConfigMap.hpp"
#include <map>
#include <vector>
#include <string>

namespace strus
{

class PatternFeatureContext
{
public:
	explicit PatternFeatureContext( const PatternFeatureConfig* config_);
	~PatternFeatureContext(){}

	const PatternFeatureConfig* config() const	{return m_config;}

	std::string normalize( const std::string& value);

private:
	typedef Reference<NormalizerFunctionContextInterface> NormalizerReference;

private:
	const PatternFeatureConfig* m_config;
	std::vector<NormalizerReference> m_normalizerlist;
};


/// \brief Set of configured features
class PatternFeatureContextMap
{
public:
	PatternFeatureContextMap()
		:m_map(0){}
	PatternFeatureContextMap( const PatternFeatureConfigMap& configMap);
	PatternFeatureContextMap( const PatternFeatureContextMap& o)
		:m_ar(o.m_ar),m_map(o.m_map){}

	PatternFeatureContext* getContext( const std::string& patternTypeName);

private:
	std::vector<PatternFeatureContext> m_ar;
	const std::map<std::string,std::size_t>* m_map;
};

}//namespace
#endif

