/*
 * Copyright (c) 2016 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "patternFeatureConfigMap.hpp"
#include "featureConfig.hpp"
#include "featureConfigMap.hpp"
#include "private/internationalization.hpp"
#include "strus/base/string_conv.hpp"

using namespace strus;

static void freeNormalizers( const std::vector<NormalizerFunctionInstanceInterface*>& normalizers)
{
	std::vector<NormalizerFunctionInstanceInterface*>::const_iterator
		ci = normalizers.begin(), ce = normalizers.end();
	for (; ci != ce; ++ci)
	{
		delete *ci;
	}
}

void PatternFeatureConfigMap::defineFeature(
	FeatureClass featureClass,
	const std::string& name,
	const std::string& patternTypeName,
	const std::vector<NormalizerFunctionInstanceInterface*>& normalizers,
	const analyzer::FeatureOptions& options)
{
	try
	{
		m_ar.reserve( m_ar.size()+1);
		m_map.insert( typename PatternNameConfigMap::value_type( patternTypeName, m_ar.size()));
		m_ar.push_back( PatternFeatureConfig( string_conv::tolower( name), normalizers, featureClass, options));
	}
	catch (const std::bad_alloc&)
	{
		freeNormalizers( normalizers);
		throw std::runtime_error( _TXT("memory allocation error defining feature"));
	}
	catch (const std::runtime_error& err)
	{
		freeNormalizers( normalizers);
		throw strus::runtime_error( _TXT("error defining feature: '%s'"), err.what());
	}
}

std::vector<const PatternFeatureConfig*> PatternFeatureConfigMap::getConfigs( const std::string& patternTypeName) const
{
	std::vector<const PatternFeatureConfig*> rt;
	std::pair<PatternNameConfigMap::const_iterator,PatternNameConfigMap::const_iterator> range = m_map.equal_range( patternTypeName);
	PatternNameConfigMap::const_iterator ci = range.first, ce = range.second;
	for (; ci != ce; ++ci)
	{
		rt.push_back( &m_ar[ ci->second]);
	}
	return rt;
}

