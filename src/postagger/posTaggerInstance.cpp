/*
 * Copyright (c) 2018 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Implementation of interface to define a POS tagger instance for creating the input for POS tagging to build the data and to create to context for tagging with the data build from the POS tagging output
/// \file posTaggerInstance.cpp
#include "posTaggerInstance.hpp"
#include "strus/lib/markup_std.hpp"
#include "strus/tokenMarkupInstanceInterface.hpp"
#include "strus/tokenMarkupContextInterface.hpp"
#include "strus/posTaggerDataInterface.hpp"
#include "strus/segmenterInstanceInterface.hpp"
#include "strus/segmenterContextInterface.hpp"
#include "strus/segmenterInterface.hpp"
#include "strus/errorBufferInterface.hpp"
#include "strus/debugTraceInterface.hpp"
#include "strus/base/local_ptr.hpp"
#include "private/errorUtils.hpp"
#include "private/internationalization.hpp"
#include <algorithm>

#define COMPONENT_NAME "POS tagger instance"
#define STRUS_DBGTRACE_COMPONENT_NAME "postag"

using namespace strus;

PosTaggerInstance::PosTaggerInstance( const SegmenterInterface* segmenter_, const analyzer::SegmenterOptions& opts, ErrorBufferInterface* errorhnd_)
	:m_errorhnd( errorhnd_),m_debugtrace(0),m_segmenter(segmenter_->createInstance( opts)),m_markup(strus::createTokenMarkupInstance_standard(errorhnd_)),m_values()
{
	if (!m_markup) {cleanup(); throw std::runtime_error( m_errorhnd->fetchError());}
	if (!m_segmenter) {cleanup(); throw std::runtime_error( m_errorhnd->fetchError());}
	DebugTraceInterface* dbg = m_errorhnd->debugTrace();
	if (dbg) m_debugtrace = dbg->createTraceContext( STRUS_DBGTRACE_COMPONENT_NAME);
}

void PosTaggerInstance::cleanup()
{
	if (m_markup) delete m_markup;
	if (m_segmenter) delete m_segmenter;
	if (m_debugtrace) delete m_debugtrace;
}

PosTaggerInstance::~PosTaggerInstance()
{
	cleanup();
}

void PosTaggerInstance::addContentExpression( const std::string& expression)
{
	m_segmenter->defineSelectorExpression( 1, expression);
}

void PosTaggerInstance::addPosTaggerInputPunctuation( const std::string& expression, const std::string& value)
{
	int idx = 1;
	std::vector<std::string>::const_iterator vi = std::find( m_values.begin(), m_values.end(), value);
	if (vi != m_values.end())
	{
		idx = vi - m_values.begin() + 2;
	}
	else
	{
		idx = m_values.size() + 2;
		m_values.push_back( value);
	}
	m_segmenter->defineSelectorExpression( idx, expression);
}

static void printContent( std::string& res, const char* segment, std::size_t segmentsize)
{
	char const* si = segment;
	const char* se = si + segmentsize;
	for (; si != se; ++si)
	{
		if ((unsigned char)*si <= 32)
		{
			if (!res.empty() && res.back() == ' ') continue;
			res.push_back( ' ');
		}
		else
		{
			res.push_back( *si);
		}
	}
}

std::string PosTaggerInstance::getPosTaggerInput( const analyzer::DocumentClass& dclass, const std::string& content) const
{
	try
	{
		strus::local_ptr<SegmenterContextInterface> segctx( m_segmenter->createContext( dclass));
		if (!segctx.get()) throw std::runtime_error( m_errorhnd->fetchError());

		segctx->putInput( content.c_str(), content.size(), true/*eof*/);
		int id = 0;
		SegmenterPosition pos = 0;
		const char* segment = 0;
		std::size_t segmentsize = 0;
		std::string rt;

		while (segctx->getNext( id, pos, segment, segmentsize))
		{
			if (id > 1)
			{
				const std::string& mrk = m_values[ id-2];
				if (rt.size() < mrk.size()) continue;

				if (0!=std::memcmp( rt.c_str() + rt.size() - mrk.size(), mrk.c_str(), mrk.size()))
				{
					rt.append( m_values[ id-2]);
				}
			}
			else
			{
				printContent( rt, segment, segmentsize);
			}
		}
		if (m_errorhnd->hasError()) return std::string();
		return rt;
	}
	CATCH_ERROR_ARG1_MAP_RETURN( _TXT("error mapping to input in \"%s\": %s"), COMPONENT_NAME, *m_errorhnd, std::string());
}

std::string PosTaggerInstance::markupDocument( const PosTaggerDataInterface* data, int docno, const analyzer::DocumentClass& dclass, const std::string& content) const
{
	try
	{
		strus::local_ptr<SegmenterContextInterface> segctx( m_segmenter->createContext( dclass));
		strus::local_ptr<TokenMarkupContextInterface> markupContext( m_markup->createContext( m_segmenter));
		if (!segctx.get()) throw std::runtime_error( m_errorhnd->fetchError());

		segctx->putInput( content.c_str(), content.size(), true/*eof*/);
		int id = 0;
		int docitr = 0;
		SegmenterPosition pos = 0;
		const char* segment = 0;
		std::size_t segmentsize = 0;
		std::string rt;

		while (segctx->getNext( id, pos, segment, segmentsize))
		{
			if (id == 1)
			{
				data->markupSegment( markupContext.get(), docno, docitr, pos, segment, segmentsize);
			}
		}
		rt.append( markupContext->markupDocument( dclass, content));
		if (m_errorhnd->hasError()) return std::string();
		return rt;
	}
	CATCH_ERROR_ARG1_MAP_RETURN( _TXT("error mapping to input in \"%s\": %s"), COMPONENT_NAME, *m_errorhnd, std::string());
}

