/*
 * Copyright (c) 2016 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef _STRUS_DOCUMENT_ANALYZER_PATTERN_FEATURE_CONFIG_MAP_HPP_INCLUDED
#define _STRUS_DOCUMENT_ANALYZER_PATTERN_FEATURE_CONFIG_MAP_HPP_INCLUDED
#include "patternFeatureConfig.hpp"
#include <map>
#include <vector>
#include <string>

namespace strus
{

/// \brief Set of configured features
class PatternFeatureConfigMap
{
public:
	PatternFeatureConfigMap()
		:m_ar(),m_minPriority(std::numeric_limits<int>::max()){}
	~PatternFeatureConfigMap(){}

	void defineFeature(
		FeatureClass featureClass,
		const std::string& name,
		const std::string& patternTypeName,
		const std::vector<NormalizerFunctionInstanceInterface*>& normalizers,
		int priority,
		const analyzer::FeatureOptions& options);

	std::vector<const PatternFeatureConfig*> getConfigs( const std::string& patternTypeName) const;
	int minPriority() const				{return m_minPriority;}

private:
	typedef std::multimap<std::string,std::size_t> PatternNameConfigMap;
	std::vector<PatternFeatureConfig> m_ar;
	PatternNameConfigMap m_map;
	int m_minPriority;
};

}//namespace
#endif


