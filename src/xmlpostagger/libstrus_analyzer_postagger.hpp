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
#include "strus/analyzer/documentClass.hpp"
#include "strus/analyzer/functionView.hpp"
#include <string>

/// \brief strus toplevel namespace
namespace strus
{
