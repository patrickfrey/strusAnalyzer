/*
 * Copyright (c) 2018 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Implementation of the introspection for the document analyzer
/// \file documentAnalyzerIntrospection.cpp
#include "strus/segmenterInstanceInterface.hpp"
#include "documentAnalyzer.hpp"
#include "private/internationalization.hpp"
#include "private/errorUtils.hpp"

using namespace strus;

class StatisticsConfigIntrospection
	:public IntrospectionInterface
{
public:
	StatisticsConfigIntrospection( const DocumentAnalyzer::StatisticsConfig* aggregator_, ErrorBufferInterface* errorhnd_)
		:m_aggregator(aggregator_),m_errorhnd(errorhnd_){}

	virtual ~StatisticsConfigIntrospection(){}
	virtual IntrospectionInterface* open( const std::string& name) const
	{
		try
		{
			if (name == "name")
			{
				return new AtomicTypeIntrospection<std::string>( &m_aggregator->name(), m_errorhnd);
			}
			else if (name == "function")
			{
				return m_aggregator->statfunc()->createIntrospection();
			}
			else
			{
				return NULL;
			}
		}
		CATCH_ERROR_MAP_RETURN( _TXT("error creating introspection: %s"), *m_errorhnd, NULL);
	}
	virtual std::string value() const
	{
		return std::string();
	}
	virtual std::vector<std::string> list() const
	{
		static const char* ar[] = {"name","function",0};
		return strus::getIntrospectionElementList( ar, m_errorhnd);
	}
	static IntrospectionInterface* constructor( const void* self, ErrorBufferInterface* errorhnd)
	{
		try
		{
			return new StatisticsConfigIntrospection( (const DocumentAnalyzer::StatisticsConfig*)self, errorhnd);
		}
		CATCH_ERROR_MAP_RETURN( _TXT("error creating introspection: %s"), *errorhnd, NULL);
	}

private:
	const DocumentAnalyzer::StatisticsConfig* m_aggregator;
	ErrorBufferInterface* m_errorhnd;
};


DocumentAnalyzerIntrospection::DocumentAnalyzerIntrospection( const DocumentAnalyzer* analyzer_, class ErrorBufferInterface* errorhnd_)
	:m_analyzer(analyzer_),m_errorhnd(errorhnd_)
{}

DocumentAnalyzerIntrospection::~DocumentAnalyzerIntrospection()
{}

IntrospectionInterface* createSegmenterInstanceIntrospection( const void* self_, class ErrorBufferInterface* errorhnd_)
{
	const SegmenterInstanceInterface* segmenter = *(const SegmenterInstanceInterface**)self_;
	return segmenter->createIntrospection();
}

static IntrospectionInterface* createSubSegmenterDefIntrospection( const void* self_, class ErrorBufferInterface* errorhnd)
{
	const DocumentAnalyzer::SubSegmenterDef* self = (const DocumentAnalyzer::SubSegmenterDef*)self_;
	class Description :public StructTypeIntrospectionDescription<DocumentAnalyzer::SubSegmenterDef>{
	public:
		Description()
		{
			(*this)
			( "documentclass", &DocumentAnalyzer::SubSegmenterDef::documentClass, &DocumentClassIntrospection::constructor)
			( "segmenter", &DocumentAnalyzer::SubSegmenterDef::segmenterInstance, createSegmenterInstanceIntrospection)
			( "selectorprefix", &DocumentAnalyzer::SubSegmenterDef::selectorPrefix, AtomicTypeIntrospection<std::string>::constructor)
			;
		}
	};
	static const Description descr;
	try
	{
		return new StructTypeIntrospection<DocumentAnalyzer::SubSegmenterDef>( self, &descr, errorhnd);
	}
	CATCH_ERROR_MAP_RETURN( _TXT("error creating introspection: %s"), *errorhnd, NULL);
}

IntrospectionInterface* DocumentAnalyzerIntrospection::open( const std::string& name) const
{
	try
	{
		if (name == "segmenter")
		{
			return m_analyzer->segmenter()->createIntrospection();
		}
		else if (name == "subsegmenter")
		{
			return (*VectorTypeIntrospection<std::vector<DocumentAnalyzer::SubSegmenterDef> >::constructor)( &m_analyzer->subsegmenterList(), m_errorhnd, createSubSegmenterDefIntrospection);
		}
		else if (name == "feature")
		{
			return strus::createFeatureConfigMapIntrospection( &m_analyzer->featureConfigMap(), m_errorhnd);
		}
		else if (name == "preprocpattern")
		{
			return NULL;
		}
		else if (name == "postprocpattern")
		{
			return NULL;
		}
		else if (name == "patternfeature")
		{
			return NULL;
		}
		else if (name == "aggregator")
		{
			return (*VectorTypeIntrospection<std::vector<DocumentAnalyzer::StatisticsConfig> >::constructor)( &m_analyzer->statisticsConfigs(), m_errorhnd, StatisticsConfigIntrospection::constructor);
		}
		else
		{
			return NULL;
		}
	}
	CATCH_ERROR_MAP_RETURN( _TXT("error creating introspection: %s"), *m_errorhnd, NULL);
}

std::string DocumentAnalyzerIntrospection::value() const
{
	return std::string();
}

std::vector<std::string> DocumentAnalyzerIntrospection::list() const
{
	static const char* ar[] = {"segmenter","subsegmenter","feature","preprocpattern","postprocpattern","patternfeature","aggregator",0};
	return strus::getIntrospectionElementList( ar, m_errorhnd);
}


