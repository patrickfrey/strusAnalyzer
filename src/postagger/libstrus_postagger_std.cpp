/*
 * Copyright (c) 2018 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Library for tagging documents with part of speech tagging info fed from a 3rdParty source (e.g. standford POS tagger, google syntaxnet, etc.)
/// \file libstrus_analyzer_postagger.hpp
#ifndef _STRUS_ANALYZER_LIB_POS_TAGGER_HPP_INCLUDED
#define _STRUS_ANALYZER_LIB_POS_TAGGER_HPP_INCLUDED
#include "strus/posTaggerInterface.hpp"
#include "strus/posTaggerInstanceInterface.hpp"
#include "strus/posTaggerContextInterface.hpp"
#include "strus/posTaggerDataInterface.hpp"
#include "strus/errorBufferInterface.hpp"
#include "strus/textProcessorInterface.hpp"
#include "strus/tokenizerFunctionInterface.hpp"
#include "strus/tokenizerFunctionInstanceInterface.hpp"
#include "strus/tokenizerFunctionContextInterface.hpp"
#include <string>

/// \brief strus toplevel namespace
using namespace strus;

PosTaggerDataInterface* strus::createPosTaggerData( const TextProcessorInterface* textproc, const std::string& tokenizerfunc, const std::vector<std::string>& tokenizerarg, ErrorBufferInterface* errorhnd)
{
	
}

PosTaggerInterface* strus::createPosTagger( const TextProcessorInterface* textproc, const std::string& tokenizerfunc, const std::vector<std::string>& tokenizerarg, ErrorBufferInterface* errorhnd)
{
	
}


