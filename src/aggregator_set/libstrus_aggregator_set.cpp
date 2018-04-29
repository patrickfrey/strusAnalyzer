/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "strus/lib/aggregator_set.hpp"
#include "strus/aggregatorFunctionInterface.hpp"
#include "strus/aggregatorFunctionInstanceInterface.hpp"
#include "strus/errorBufferInterface.hpp"
#include "strus/analyzer/documentTerm.hpp"
#include "strus/analyzer/functionView.hpp"
#include "strus/base/dll_tags.hpp"
#include "strus/base/string_conv.hpp"
#include "strus/base/introspection.hpp"
#include "private/errorUtils.hpp"
#include "private/internationalization.hpp"
#include <vector>
#include <string>
#include <map>
#include <cmath>

using namespace strus;
using namespace strus::analyzer;

static bool g_intl_initialized = false;

#define MODULE_NAME "typeset/valueset aggregator"

class SetAggregatorFunctionInstance
	:public AggregatorFunctionInstanceInterface
{
public:
	/// \brief Constructor
	SetAggregatorFunctionInstance( const std::string& type, const std::vector<std::string>& items, ErrorBufferInterface* errorhnd)
		:m_type(string_conv::tolower(type)),m_itemmap(),m_errorhnd(errorhnd)
	{
		if (items.size() >= (8 * sizeof(unsigned int)))
		{
			throw strus::runtime_error(_TXT("too many elements to build a set represented as bitfield for the %s"), MODULE_NAME);
		}
		std::vector<std::string>::const_iterator
			fi = items.begin(), fe = items.end();
		for (unsigned int fidx=0; fi != fe; ++fi,++fidx)
		{
			m_itemmap[ *fi] = 1 << fidx;
		}
	}

	virtual NumericVariant evaluate( const analyzer::Document& document) const
	{
		try
		{
			unsigned int rt = 0;
			std::vector<DocumentTerm>::const_iterator
				si = document.searchIndexTerms().begin(),
				se = document.searchIndexTerms().end();
	
			if (m_type.empty())
			{
				for (; si != se; ++si)
				{
					std::map<std::string,unsigned int>::const_iterator
						fi = m_itemmap.find( si->type());
					if (fi != m_itemmap.end())
					{
						rt |= fi->second;
					}
				}
			}
			else
			{
				for (; si != se; ++si)
				{
					if (si->type() == m_type)
					{
						std::map<std::string,unsigned int>::const_iterator
							fi = m_itemmap.find( si->value());
						if (fi != m_itemmap.end())
						{
							rt |= fi->second;
						}
					}
				}
			}
			return NumericVariant( (NumericVariant::IntType)rt);
		}
		CATCH_ERROR_ARG1_MAP_RETURN( _TXT("error in '%s': %s"), MODULE_NAME, *m_errorhnd, (NumericVariant::IntType)0);
	}

	virtual analyzer::FunctionView view() const
	{
		try
		{
			return analyzer::FunctionView( "set")
				( "featuretype", m_type)
				( "set", m_itemmap)
			;
		}
		CATCH_ERROR_MAP_RETURN( _TXT("error in introspection: %s"), *m_errorhnd, analyzer::FunctionView());
	}

	virtual IntrospectionInterface* createIntrospection() const
	{
		class Description :public StructTypeIntrospectionDescription<SetAggregatorFunctionInstance>{
		public:
			Description()
			{
				(*this)
				["set"]
				( "type", &SetAggregatorFunctionInstance::m_type, AtomicTypeIntrospection<std::string>::constructor)
				( "items", &SetAggregatorFunctionInstance::m_itemmap, AtomicMapTypeIntrospection<ItemMap>::constructor);
			}
		};
		static const Description descr;
		try
		{
			return new StructTypeIntrospection<SetAggregatorFunctionInstance>( this, &descr, m_errorhnd);
		}
		CATCH_ERROR_ARG1_MAP_RETURN( _TXT("error in introspection of '%s': %s"), MODULE_NAME, *m_errorhnd, NULL);
	}

private:
	std::string m_type;
	typedef std::map<std::string,unsigned int> ItemMap;
	ItemMap m_itemmap;
	ErrorBufferInterface* m_errorhnd;
};

class TypeSetAggregatorFunction
	:public AggregatorFunctionInterface
{
public:
	explicit TypeSetAggregatorFunction( ErrorBufferInterface* errorhnd_)
		:m_errorhnd(errorhnd_){}

	virtual AggregatorFunctionInstanceInterface* createInstance( const std::vector<std::string>& args) const
	{
		if (args.size() == 0)
		{
			m_errorhnd->report( ErrorCodeIncompleteDefinition, _TXT("at least one feature type name expected as argument for '%s' function"), MODULE_NAME);
			return 0;
		}
		try
		{
			return new SetAggregatorFunctionInstance( std::string(), args, m_errorhnd);
		}
		CATCH_ERROR_ARG1_MAP_RETURN( _TXT("error in '%s': %s"), MODULE_NAME, *m_errorhnd, 0);
	}

	virtual const char* getDescription() const
	{
		return _TXT("aggregator building a set of features types that exist in the document (represented as bit-field)");
	}

private:
	const char* m_description;
	ErrorBufferInterface* m_errorhnd;
};

class ValueSetAggregatorFunction
	:public AggregatorFunctionInterface
{
public:
	explicit ValueSetAggregatorFunction( ErrorBufferInterface* errorhnd_)
		:m_errorhnd(errorhnd_){}

	virtual AggregatorFunctionInstanceInterface* createInstance( const std::vector<std::string>& args) const
	{
		if (args.size() < 2)
		{
			m_errorhnd->report( ErrorCodeIncompleteDefinition, _TXT("at least one feature type name and a value expected as argument for '%s' function"), MODULE_NAME);
			return 0;
		}
		try
		{
			return new SetAggregatorFunctionInstance( args[0], std::vector<std::string>( args.begin()+1, args.end()), m_errorhnd);
		}
		CATCH_ERROR_ARG1_MAP_RETURN( _TXT("error in '%s': %s"), MODULE_NAME, *m_errorhnd, 0);
	}

	virtual const char* getDescription() const
	{
		return _TXT("aggregator building a set of features values that exist in the document (represented as bit-field)");
	}

private:
	const char* m_description;
	ErrorBufferInterface* m_errorhnd;
};


DLL_PUBLIC AggregatorFunctionInterface* strus::createAggregator_typeset( ErrorBufferInterface* errorhnd)
{
	try
	{
		if (!g_intl_initialized)
		{
			strus::initMessageTextDomain();
			g_intl_initialized = true;
		}
		return new TypeSetAggregatorFunction( errorhnd);
	}
	CATCH_ERROR_ARG1_MAP_RETURN( _TXT("cannot create '%s': %s"), "aggregator typeset", *errorhnd, 0);
}

DLL_PUBLIC AggregatorFunctionInterface* strus::createAggregator_valueset( ErrorBufferInterface* errorhnd)
{
	try
	{
		if (!g_intl_initialized)
		{
			strus::initMessageTextDomain();
			g_intl_initialized = true;
		}
		return new ValueSetAggregatorFunction( errorhnd);
	}
	CATCH_ERROR_ARG1_MAP_RETURN( _TXT("cannot create '%s': %s"), "aggregator valueset", *errorhnd, 0);
}


