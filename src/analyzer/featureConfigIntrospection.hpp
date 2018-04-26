/*
 * Copyright (c) 2018 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Implementation of the introspection for the analyzer feature configuration
/// \file featureConfigIntrospection.hpp
#ifndef _STRUS_ANALYZER_DOCUMENT_FEATURE_CONFIG_INTROSPECTION_HPP_INCLUDED
#define _STRUS_ANALYZER_DOCUMENT_FEATURE_CONFIG_INTROSPECTION_HPP_INCLUDED
#include "strus/introspectionInterface.hpp"
#include "strus/errorBufferInterface.hpp"

namespace strus
{
/// \brief Forward declaration
class FeatureConfig;
/// \brief Forward declaration
class FeatureConfigMap;

class FeatureConfigIntrospection
	:public IntrospectionInterface
{
public:
	FeatureConfigIntrospection( const FeatureConfig* featureConfig_, ErrorBufferInterface* errorhnd_)
		:m_featureConfig(featureConfig_),m_errorhnd(errorhnd_){}
	virtual ~FeatureConfigIntrospection( ){}

	virtual IntrospectionInterface* open( const std::string& name) const;
	virtual std::string value() const;

	virtual std::vector<std::string> list() const;
	static IntrospectionInterface* constructor( const void* self_, class ErrorBufferInterface* errorhnd_);

private:
	const FeatureConfig* m_featureConfig;
	mutable ErrorBufferInterface* m_errorhnd;
};

IntrospectionInterface* createFeatureConfigMapIntrospection( const FeatureConfigMap* map, ErrorBufferInterface* errorhnd);

}//namespace
#endif


