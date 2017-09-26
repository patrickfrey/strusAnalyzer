/*
* Copyright (c) 2014 Patrick P. Frey
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
/// \brief Library with some convenient functions to create analyzer objects
/// \file "libstrus_analyzer_objbuild.cpp"
#include "strus/lib/analyzer_objbuild.hpp"
#include "strus/lib/textproc.hpp"
#include "strus/lib/analyzer.hpp"
#include "strus/lib/segmenter_textwolf.hpp"
#include "strus/lib/segmenter_cjson.hpp"
#include "strus/lib/segmenter_tsv.hpp"
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
		:m_errorhnd(errorhnd_)
		,m_textproc(strus::createTextProcessor(errorhnd_))
		,m_segmenter_textwolf(createSegmenter_textwolf(errorhnd_))
		,m_segmenter_cjson(createSegmenter_cjson(errorhnd_))
		,m_segmenter_tsv(createSegmenter_tsv(errorhnd_))
	{}

	/// \brief Destructor
	virtual ~AnalyzerObjectBuilder(){}

	virtual const TextProcessorInterface* getTextProcessor() const
	{
		return m_textproc.get();
	}

	virtual const SegmenterInterface* getSegmenter( const std::string& segmenterName) const
	{
		try
		{
			if (segmenterName.empty() || utils::caseInsensitiveEquals( segmenterName, "textwolf"))
			{
				return m_segmenter_textwolf.get();
			}
			else if (utils::caseInsensitiveEquals( segmenterName, "cjson"))
			{
				return m_segmenter_cjson.get();
			}
			else if (utils::caseInsensitiveEquals( segmenterName, "tsv"))
			{
				return m_segmenter_tsv.get();
			}
			else
			{
				throw strus::runtime_error(_TXT("unknown document segmenter name: '%s'"), segmenterName.c_str());
			}
		}
		CATCH_ERROR_MAP_RETURN( _TXT("failed to get document segmenter: '%s'"), *m_errorhnd, 0);
	}

	virtual const SegmenterInterface* findMimeTypeSegmenter( const std::string& mimetype) const
	{
		try
		{
			if (utils::caseInsensitiveEquals( mimetype, m_segmenter_textwolf->mimeType()))
			{
				return m_segmenter_textwolf.get();
			}
			else if (utils::caseInsensitiveEquals( mimetype, m_segmenter_cjson->mimeType()))
			{
				return m_segmenter_cjson.get();
			}
			else if (utils::caseInsensitiveEquals( mimetype, m_segmenter_tsv->mimeType()))
			{
				return m_segmenter_tsv.get();
			}
			else
			{
				throw strus::runtime_error(_TXT("unknown document mime type: '%s'"), mimetype.c_str());
			}
		}
		CATCH_ERROR_MAP_RETURN( _TXT("failed to get document segmenter: '%s'"), *m_errorhnd, 0);
	}

	virtual DocumentAnalyzerInterface* createDocumentAnalyzer( const SegmenterInterface* segmenter, const analyzer::SegmenterOptions& opts) const
	{
		return strus::createDocumentAnalyzer( m_textproc.get(), segmenter, opts, m_errorhnd);
	}

	virtual QueryAnalyzerInterface* createQueryAnalyzer() const
	{
		return strus::createQueryAnalyzer( m_errorhnd);
	}

private:
	ErrorBufferInterface* m_errorhnd;
	Reference<TextProcessorInterface> m_textproc;
	Reference<SegmenterInterface> m_segmenter_textwolf;
	Reference<SegmenterInterface> m_segmenter_cjson;
	Reference<SegmenterInterface> m_segmenter_tsv;
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


