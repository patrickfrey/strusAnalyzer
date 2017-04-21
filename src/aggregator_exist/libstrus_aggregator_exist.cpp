/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "strus/lib/aggregator_exist.hpp"
#include "strus/aggregatorFunctionInterface.hpp"
#include "strus/aggregatorFunctionInstanceInterface.hpp"
#include "strus/errorBufferInterface.hpp"
#include "strus/analyzer/term.hpp"
#include "strus/base/dll_tags.hpp"
#include "private/errorUtils.hpp"
#include "private/internationalization.hpp"
#include "private/utils.hpp"
#include <vector>
#include <string>
#include <map>
#include <cmath>

using namespace strus;
using namespace strus::analyzer;

static bool g_intl_initialized = false;

#define MODULE_NAME "exist aggregator"

class ExistAggregatorFunctionInstance
	:public AggregatorFunctionInstanceInterface
{
public:
	/// \brief Constructor
	ExistAggregatorFunctionInstance( const std::vector<std::string>& featuretypes, ErrorBufferInterface* errorhnd)
		:m_featuretypemap(),m_errorhnd(errorhnd)
	{
		if (featuretypes.size() >= (8 * sizeof(unsigned int)))
		{
			throw strus::runtime_error(_TXT("too many elements to build a set represented as bitfield for the %s"), MODULE_NAME);
		}
		std::vector<std::string>::const_iterator
			fi = featuretypes.begin(), fe = featuretypes.end();
		for (unsigned int fidx=0; fi != fe; ++fi,++fidx)
		{
			m_featuretypemap[ *fi] = 1 << fidx;
		}
	}

	virtual NumericVariant evaluate( const analyzer::Document& document) const
	{
		try
		{
			unsigned int rt = 0;
			std::vector<Term>::const_iterator
				si = document.searchIndexTerms().begin(),
				se = document.searchIndexTerms().end();
	
			for (; si != se; ++si)
			{
				std::map<std::string,unsigned int>::const_iterator fi = m_featuretypemap.find( si->type());
				if (fi != m_featuretypemap.end())
				{
					rt |= fi->second;
				}
			}
			return NumericVariant( rt);
		}
		CATCH_ERROR_MAP_ARG1_RETURN( _TXT("error in '%s': %s"), MODULE_NAME, *m_errorhnd, 0);
	}

private:
	std::map<std::string,unsigned int> m_featuretypemap;
	ErrorBufferInterface* m_errorhnd;
};

class ExistAggregatorFunction
	:public AggregatorFunctionInterface
{
public:
	explicit ExistAggregatorFunction( ErrorBufferInterface* errorhnd_)
		:m_errorhnd(errorhnd_){}

	virtual AggregatorFunctionInstanceInterface* createInstance( const std::vector<std::string>& args) const
	{
		if (args.size() == 0)
		{
			m_errorhnd->report( _TXT("at least one feature type name as argument expected for '%s' function"), MODULE_NAME);
			return 0;
		}
		try
		{
			return new ExistAggregatorFunctionInstance( args, m_errorhnd);
		}
		CATCH_ERROR_MAP_ARG1_RETURN( _TXT("error in '%s': %s"), MODULE_NAME, *m_errorhnd, 0);
	}

	virtual const char* getDescription() const
	{
		return _TXT("aggregator building a set of features types that exist in the document (represented as bit-field)");
	}

private:
	const char* m_description;
	ErrorBufferInterface* m_errorhnd;
};


DLL_PUBLIC AggregatorFunctionInterface* strus::createAggregator_exist( ErrorBufferInterface* errorhnd)
{
	try
	{
		if (!g_intl_initialized)
		{
			strus::initMessageTextDomain();
			g_intl_initialized = true;
		}
		return new ExistAggregatorFunction( errorhnd);
	}
	CATCH_ERROR_MAP_ARG1_RETURN( _TXT("cannot create '%s': %s"), MODULE_NAME, *errorhnd, 0);
}



