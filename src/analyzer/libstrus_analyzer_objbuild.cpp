/*
* Copyright (c) 2014 Patrick P. Frey
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
/// \brief Library with some convenient functions to create analyzer objects
/// \file "analyzer_objbuild.cpp"
#include "strus/lib/analyzer_objbuild.hpp"
#include "strus/lib/textproc.hpp"
#include "strus/lib/analyzer.hpp"
#include "strus/lib/segmenter_textwolf.hpp"
#include "strus/reference.hpp"
#include "strus/errorBufferInterface.hpp"
#include "strus/analyzerObjectBuilderInterface.hpp"
#include "strus/textProcessorInterface.hpp"
#include "strus/segmenterInterface.hpp"
#include "strus/documentAnalyzerInterface.hpp"
#include "strus/queryAnalyzerInterface.hpp"
#include "strus/base/dll_tags.hpp"
#include "private/internationalization.hpp"
#include "private/errorUtils.hpp"
#include "private/utils.hpp"
#include "strus/base/configParser.hpp"
#include <memory>

using namespace strus;

static bool g_intl_initialized = false;

/// \brief Interface providing a mechanism to create complex multi component objects for the document and query analysis in strus.
class AnalyzerObjectBuilder
	:public AnalyzerObjectBuilderInterface
{
public:
	explicit AnalyzerObjectBuilder( ErrorBufferInterface* errorhnd_)
		:m_errorhnd(errorhnd_),m_textproc(strus::createTextProcessor(errorhnd_)){}
	/// \brief Destructor
	virtual ~AnalyzerObjectBuilder(){}

	virtual const TextProcessorInterface* getTextProcessor() const
	{
		return m_textproc.get();
	}

	virtual SegmenterInterface* createSegmenter( const std::string& segmenterName=std::string()) const
	{
		try
		{
			if (segmenterName.empty() || utils::caseInsensitiveEquals( segmenterName, "textwolf"))
			{
				return createSegmenter_textwolf( m_errorhnd);
			}
			else
			{
				throw strus::runtime_error(_TXT("unknown document segmenter: '%s'"), segmenterName.c_str());
			}
		}
		CATCH_ERROR_MAP_RETURN( _TXT("error creating document segmenter: %s"), *m_errorhnd, 0);
	}

	virtual DocumentAnalyzerInterface* createDocumentAnalyzer( SegmenterInterface* segmenter) const
	{
		return strus::createDocumentAnalyzer( segmenter, m_errorhnd);
	}

	virtual QueryAnalyzerInterface* createQueryAnalyzer() const
	{
		return strus::createQueryAnalyzer( m_errorhnd);
	}

private:
	ErrorBufferInterface* m_errorhnd;
	Reference<TextProcessorInterface> m_textproc;
};


DLL_PUBLIC AnalyzerObjectBuilderInterface*
	strus::createAnalyzerObjectBuilder_default(
		ErrorBufferInterface* errorhnd)
{
	try
	{
		if (!g_intl_initialized)
		{
			strus::initMessageTextDomain();
			g_intl_initialized = true;
		}
		return new AnalyzerObjectBuilder( errorhnd);
	}
	CATCH_ERROR_MAP_RETURN( _TXT("cannot create analyzer object builder: %s"), *errorhnd, 0);
}


