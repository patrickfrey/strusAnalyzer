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
#include "strus/analyzer/functionView.hpp"
#include <string>

namespace strus
{

/// \brief Forward declaration
class TokenMarkupContextInterface;
/// \brief Forward declaration
class SegmenterMarkupContextInterface;
/// \brief Forward declaration
class IntrospectionInterface;

/// \brief Interface for building the automaton for detecting patterns of tokens in a document stream
class TokenMarkupInstanceInterface
{
public:
	/// \brief Destructor
	virtual ~TokenMarkupInstanceInterface(){}

	/// \brief Create the context to markup tokens or spans in a document
	/// \return the token markup context
	/// \remark The context cannot be reset. So the context has to be recreated for every processed unit (document)
	virtual TokenMarkupContextInterface* createContext() const=0;

	/// \brief Get the definition of the function as structure for introspection
	/// \return structure for introspection
	virtual analyzer::FunctionView view() const=0;

	/// \brief Create an interface for introspection
	/// \return introspection interface (with ownership)
	virtual IntrospectionInterface* createIntrospection() const=0;
};

} //namespace
#endif


