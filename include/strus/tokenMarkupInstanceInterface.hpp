/*
 * Copyright (c) 2016 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Interface for putting annotations into text documents
/// \file "tokenMarkupInstanceInterface.hpp"
#ifndef _STRUS_ANALYZER_TOKEN_MARKUP_INSTANCE_INTERFACE_HPP_INCLUDED
#define _STRUS_ANALYZER_TOKEN_MARKUP_INSTANCE_INTERFACE_HPP_INCLUDED
#include "strus/structView.hpp"
#include <string>

namespace strus
{

/// \brief Forward declaration
class TokenMarkupContextInterface;
/// \brief Forward declaration
class SegmenterMarkupContextInterface;

/// \brief Interface for building the automaton for detecting patterns of tokens in a document stream
class TokenMarkupInstanceInterface
{
public:
	/// \brief Destructor
	virtual ~TokenMarkupInstanceInterface(){}

	/// \brief Create the context to markup tokens or spans in a document
	/// \param[in] segmenter segmenter to use for inserting document markup tags
	/// \return the token markup context
	/// \remark The context cannot be reset. So the context has to be recreated for every processed unit (document)
	virtual TokenMarkupContextInterface* createContext( const SegmenterInstanceInterface* segmenter) const=0;

	/// \brief Get the name of the function
	/// \return the identifier
	virtual const char* name() const=0;

	/// \brief Return a structure with all definitions for introspection
	/// \return the structure with all definitions for introspection
	virtual StructView view() const=0;
};

} //namespace
#endif


