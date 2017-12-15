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
		m_map[ patternTypeName] = m_ar.size();
		m_ar.reserve( m_ar.size()+1);
		m_ar.push_back( PatternFeatureConfig( string_conv::tolower( name), normalizers, featureClass, options));
	}
	catch (const std::bad_alloc&)
	{
		freeNormalizers( normalizers);
		throw strus::runtime_error( "%s", _TXT("memory allocation error defining feature"));
	}
	catch (const std::runtime_error& err)
	{
		freeNormalizers( normalizers);
		throw strus::runtime_error( _TXT("error defining feature: '%s'"), err.what());
	}
}

const PatternFeatureConfig* PatternFeatureConfigMap::getConfig( const std::string& patternTypeName) const
{
	std::map<std::string,std::size_t>::const_iterator mi = m_map.find( patternTypeName);
	if (mi == m_map.end()) return 0;
	return &m_ar[ mi->second];
}

