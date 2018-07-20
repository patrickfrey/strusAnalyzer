/*
 * Copyright (c) 2018 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Implementation of context to markup documents with tags derived from POS tagging
/// \file posTaggerContext.cpp
#include "posTaggerContext.hpp"
#include "strus/segmenterInstanceInterface.hpp"
#include "strus/segmenterContextInterface.hpp"
#include "strus/tokenMarkupInstanceInterface.hpp"
#include "strus/tokenMarkupContextInterface.hpp"
#include "strus/posTaggerDataInterface.hpp"
#include "strus/errorBufferInterface.hpp"
#include "strus/debugTraceInterface.hpp"
#include "strus/base/local_ptr.hpp"
#include "private/internationalization.hpp"
#include "private/errorUtils.hpp"

using namespace strus;

#define COMPONENT_NAME "POS tagger instance"
#define STRUS_DBGTRACE_COMPONENT_NAME "postag"

PosTaggerContext::PosTaggerContext( const SegmenterInstanceInterface* segmenter_, const TokenMarkupInstanceInterface* markup_, PosTaggerDataInterface* data_, ErrorBufferInterface* errorhnd_)
	:m_errorhnd(errorhnd_),m_debugtrace(0),m_segmenter(segmenter_),m_markup(markup_),m_data(data_)
{
	DebugTraceInterface* dbg = m_errorhnd->debugTrace();
	if (dbg) m_debugtrace = dbg->createTraceContext( STRUS_DBGTRACE_COMPONENT_NAME);
}

PosTaggerContext::~PosTaggerContext()
{
	if (m_debugtrace) delete m_debugtrace;
}

std::string PosTaggerContext::markupDocument( int docno, const analyzer::DocumentClass& dclass, const std::string& content)
{
	try
	{
		strus::local_ptr<SegmenterContextInterface> segctx( m_segmenter->createContext( dclass));
		strus::local_ptr<TokenMarkupContextInterface> markupContext( m_markup->createContext( m_segmenter));
		if (!segctx.get()) throw std::runtime_error( m_errorhnd->fetchError());

		segctx->putInput( content.c_str(), content.size(), true/*eof*/);
		int id = 0;
		SegmenterPosition pos = 0;
		SegmenterPosition prevpos = 0;
		const char* segment = 0;
		std::size_t segmentsize = 0;
		std::string rt;

		while (segctx->getNext( id, pos, segment, segmentsize))
		{
			if (id == 1)
			{
				m_data->markupSegment( markupContext.get(), docno, pos, segment, segmentsize);
			}
		}
		rt.append( markupContext->markupDocument( dclass, content));
		if (m_errorhnd->hasError()) return std::string();
		return rt;
	}
	CATCH_ERROR_ARG1_MAP_RETURN( _TXT("error mapping to input in \"%s\": %s"), COMPONENT_NAME, *m_errorhnd, std::string());
}

