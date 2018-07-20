/*
 * Copyright (c) 2018 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Implementation of context to markup documents with tags derived from POS tagging
/// \file posTaggerContext.hpp
#ifndef _STRUS_ANALYZER_POS_TAGGER_CONTEXT_IMPLEMENTATION_HPP_INCLUDED
#define _STRUS_ANALYZER_POS_TAGGER_CONTEXT_IMPLEMENTATION_HPP_INCLUDED
#include "strus/posTaggerContextInterface.hpp"
#include <string>

/// \brief strus toplevel namespace
namespace strus
{
/// \brief Forward declaration
class SegmenterInstanceInterface;
/// \brief Forward declaration
class TokenMarkupInstanceInterface;
/// \brief Forward declaration
class PosTaggerDataInterface;
/// \brief Forward declaration
class ErrorBufferInterface;
/// \brief Forward declaration
class DebugTraceContextInterface;

/// \brief Implementation of context to markup documents with tags derived from POS tagging
class PosTaggerContext
	:public PosTaggerContextInterface
{
public:
	PosTaggerContext( const SegmenterInstanceInterface* segmenter_, const TokenMarkupInstanceInterface* markup_, PosTaggerDataInterface* data_, ErrorBufferInterface* errorhnd_);
	virtual ~PosTaggerContext();

	virtual std::string markupDocument( int docno, const analyzer::DocumentClass& dclass, const std::string& content);

private:
	ErrorBufferInterface* m_errorhnd;
	DebugTraceContextInterface* m_debugtrace;
	const SegmenterInstanceInterface* m_segmenter;
	const TokenMarkupInstanceInterface* m_markup; 
	PosTaggerDataInterface* m_data;
};

}//namespace
#endif

