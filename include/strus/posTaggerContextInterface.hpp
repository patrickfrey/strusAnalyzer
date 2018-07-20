/*
 * Copyright (c) 2018 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Context to markup documents with tags derived from POS tagging
/// \file posTaggerContextInterface.hpp
#ifndef _STRUS_ANALYZER_POS_TAGGER_CONTEXT_INTERFACE_HPP_INCLUDED
#define _STRUS_ANALYZER_POS_TAGGER_CONTEXT_INTERFACE_HPP_INCLUDED
#include "strus/analyzer/documentClass.hpp"
#include <string>

/// \brief strus toplevel namespace
namespace strus
{

/// \brief Context to markup documents with tags derived from POS tagging
class PosTaggerContextInterface
{
public:
	/// \brief Destructor
	virtual ~PosTaggerContextInterface(){}

	/// \brief Markup a document with POS tagging info
	/// \param[ín] dclass document class of document to markup
	/// \param[ín] content to markup
	virtual std::string markupDocument( int docno, const analyzer::DocumentClass& dclass, const std::string& content)=0;
};

}//namespace
#endif

