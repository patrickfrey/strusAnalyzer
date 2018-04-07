/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "featureConfigMap.hpp"
#include "private/internationalization.hpp"
#include "strus/base/string_conv.hpp"

using namespace strus;

const FeatureConfig& FeatureConfigMap::featureConfig( int featidx) const
{
	if (featidx <= 0 || (std::size_t)featidx > m_ar.size())
	{
		throw std::runtime_error( _TXT("internal: unknown index of feature"));
	}
	return m_ar[ featidx-1];
}

static void freeNormalizers( const std::vector<NormalizerFunctionInstanceInterface*>& normalizers)
{
	std::vector<NormalizerFunctionInstanceInterface*>::const_iterator
		ci = normalizers.begin(), ce = normalizers.end();
	for (; ci != ce; ++ci)
	{
		delete *ci;
	}
}

unsigned int FeatureConfigMap::defineFeature(
		FeatureClass featureClass,
		const std::string& featType,
		TokenizerFunctionInstanceInterface* tokenizer,
		const std::vector<NormalizerFunctionInstanceInterface*>& normalizers,
		const analyzer::FeatureOptions& options)
{
	try
	{
		if (m_ar.size()+1 >= MaxNofFeatures)
		{
			throw std::runtime_error( _TXT("number of features defined exceeds maximum limit"));
		}
		m_ar.reserve( m_ar.size()+1);
		m_ar.push_back( FeatureConfig( string_conv::tolower( featType), tokenizer, normalizers, featureClass, options));
		return m_ar.size();
	}
	catch (const std::bad_alloc&)
	{
		freeNormalizers( normalizers);
		delete tokenizer;
		throw std::runtime_error( _TXT("memory allocation error defining feature"));
	}
	catch (const std::runtime_error& err)
	{
		freeNormalizers( normalizers);
		delete tokenizer;
		throw strus::runtime_error( _TXT("error defining feature: '%s'"), err.what());
	}
}

