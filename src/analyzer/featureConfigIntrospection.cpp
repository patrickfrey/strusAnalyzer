/*
 * Copyright (c) 2018 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Implementation of the introspection for the analyzer feature configuration
/// \file featureConfigIntrospection.cpp
#include "featureConfigIntrospection.hpp"
#include "featureConfigMap.hpp"
#include "featureConfig.hpp"
#include "strus/base/introspection.hpp"
#include "private/internationalization.hpp"
#include "private/errorUtils.hpp"
#include "strus/analyzer/positionBind.hpp"

using namespace strus;

typedef Reference<NormalizerFunctionInstanceInterface> NormalizerReference;
typedef Reference<TokenizerFunctionInstanceInterface> TokenizerReference;

static IntrospectionInterface* normalizerReferenceIntrospectionConstructor( const void* self_, ErrorBufferInterface* errorhnd)
{
	const NormalizerReference* self = (const NormalizerReference*)self_;
	return self->get()->createIntrospection();
}

static IntrospectionInterface* featureClassIntrospectionConstructor( const FeatureClass& featureClass, ErrorBufferInterface* errorhnd)
{
	try
	{
		return new ConstIntrospection( featureClassName( featureClass), errorhnd);
	}
	catch (const std::bad_alloc&)
	{
		errorhnd->report( ErrorCodeOutOfMem, _TXT( "out of memory creafing introspection"));
		return NULL;
	}
}

static IntrospectionInterface* createPositionBindIntrospection( analyzer::PositionBind posbind, ErrorBufferInterface* errorhnd)
{
	try
	{
		const char* posBindStr = 0;
		switch (posbind)
		{
			case analyzer::BindContent: posBindStr = "content";
			case analyzer::BindSuccessor: posBindStr = "succ";
			case analyzer::BindPredecessor: posBindStr = "pred";
			case analyzer::BindUnique: posBindStr = "unique";
		}
		if (posBindStr)
		{
			return new ConstIntrospection( posBindStr, errorhnd);
		}
		return NULL;
	}
	catch (const std::bad_alloc&)
	{
		errorhnd->report( ErrorCodeOutOfMem, _TXT( "out of memory creafing introspection"));
		return NULL;
	}
}

class FeatureOptionsIntrospection
	:public IntrospectionInterface
{
public:
	FeatureOptionsIntrospection( const analyzer::FeatureOptions* fopt_, ErrorBufferInterface* errorhnd_)
		:m_fopt(fopt_),m_errorhnd(errorhnd_){}

	virtual ~FeatureOptionsIntrospection(){}
	virtual IntrospectionInterface* open( const std::string& name) const
	{
		if (name == "position")
		{
			return createPositionBindIntrospection( m_fopt->positionBind(), m_errorhnd);
		}
		else
		{
			return NULL;
		}
	}
	virtual std::string value() const
	{
		return std::string();
	}
	virtual std::vector<std::string> list() const
	{
		static const char* ar[] = {"position",0};
		return strus::getIntrospectionElementList( ar, m_errorhnd);
	}
private:
	const analyzer::FeatureOptions* m_fopt;
	ErrorBufferInterface* m_errorhnd;
};

IntrospectionInterface* FeatureConfigIntrospection::open( const std::string& name) const
{
	try
	{
		if (name == "name")
		{
			return (*AtomicTypeIntrospection<std::string>::constructor)( &m_featureConfig->name(), m_errorhnd);
		}
		else if (name == "tokenizer")
		{
			return m_featureConfig->tokenizer()->createIntrospection();
		}
		else if (name == "normalizer")
		{
			typedef std::vector<NormalizerReference> NormalizerReferenceList;
			return (*VectorTypeIntrospection<NormalizerReferenceList>::constructor)(
					&m_featureConfig->normalizerlist(), m_errorhnd, &normalizerReferenceIntrospectionConstructor);
		}
		else if (name == "class")
		{
			return featureClassIntrospectionConstructor( m_featureConfig->featureClass(), m_errorhnd);
		}
		else if (name == "options")
		{
			return new FeatureOptionsIntrospection( &m_featureConfig->options(), m_errorhnd);
		}
		else
		{
			return NULL;
		}
	}
	catch (const std::bad_alloc&)
	{
		m_errorhnd->report( ErrorCodeOutOfMem, _TXT( "out of memory creafing introspection"));
		return NULL;
	}
}

std::string FeatureConfigIntrospection::value() const
{
	return std::string();
}

std::vector<std::string> FeatureConfigIntrospection::list() const
{
	static const char* ar_index[] = {"name","tokenizer","normalizer","class","options",0};
	static const char* ar_attr[] = {"name","tokenizer","normalizer","class",0};
	switch (m_featureConfig->featureClass())
	{
		case FeatMetaData: 
		case FeatAttribute:
			return strus::getIntrospectionElementList( ar_attr, m_errorhnd);
		case FeatSearchIndexTerm:
		case FeatForwardIndexTerm:
		case FeatPatternLexem:
			return strus::getIntrospectionElementList( ar_index, m_errorhnd);
	}
	return std::vector<std::string>();
}

IntrospectionInterface* FeatureConfigIntrospection::constructor( const void* self, class ErrorBufferInterface* errorhnd)
{
	try
	{
		return new FeatureConfigIntrospection( (const FeatureConfig*)self, errorhnd);
	}
	catch (const std::bad_alloc&)
	{
		errorhnd->report( ErrorCodeOutOfMem, _TXT("out of memory"));
		return NULL;
	}
}


IntrospectionInterface* strus::createFeatureConfigMapIntrospection( const FeatureConfigMap* map, ErrorBufferInterface* errorhnd)
{
	return VectorTypeIntrospection<std::vector<FeatureConfig> >::constructor( 
				&map->list(), errorhnd, FeatureConfigIntrospection::constructor);
}

