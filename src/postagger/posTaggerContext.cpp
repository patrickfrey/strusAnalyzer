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

#error DEPRECATED
using namespace strus;

#define COMPONENT_NAME "POS tagger instance"
#define STRUS_DBGTRACE_COMPONENT_NAME "postag"

PosTaggerContext::PosTaggerContext( const SegmenterInstanceInterface* segmenter_, const TokenMarkupInstanceInterface* markup_, const PosTaggerDataInterface* data_, ErrorBufferInterface* errorhnd_)
	:m_errorhnd(errorhnd_),m_debugtrace(0),m_segmenter(segmenter_),m_markup(markup_),m_data(data_)
{
	DebugTraceInterface* dbg = m_errorhnd->debugTrace();
	if (dbg) m_debugtrace = dbg->createTraceContext( STRUS_DBGTRACE_COMPONENT_NAME);
}

PosTaggerContext::~PosTaggerContext()
{
	if (m_debugtrace) delete m_debugtrace;
}

