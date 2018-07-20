/*
 * Copyright (c) 2018 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Interface to define a POS tagger instance for creating the input for POS tagging to build the data and to create to context for tagging with the data build from the POS tagging output
/// \file posTaggerInstanceInterface.hpp
#ifndef _STRUS_ANALYZER_POS_TAGGER_INSTANCE_INTERFACE_HPP_INCLUDED
#define _STRUS_ANALYZER_POS_TAGGER_INSTANCE_INTERFACE_HPP_INCLUDED
#include "strus/analyzer/documentClass.hpp"
#include <string>

/// \brief strus toplevel namespace
namespace strus
{

/// \brief Forward declaration
class PosTaggerDataInterface;
/// \brief Forward declaration
class PosTaggerContextInterface;


/// \brief Interface to define a POS tagger instance for creating the input for POS tagging to build the data and to create to context for tagging with the data build from the POS tagging output
class PosTaggerInstanceInterface
{
public:
	/// \brief Destructor
	virtual ~PosTaggerInstanceInterface(){}

	/// \brief Defines an expression for selecting chunks from a document to do tagging
	/// \param[in] expression expression for selecting chunks
	virtual void addContentExpression( const std::string& expression)=0;

	/// \brief Defines a punktuation marker for POS tagger input
	/// \param[in] expression expression selecting the marker position
	/// \param[in] punct punctuation marker value
	virtual void addPosTaggerInputPunctuation( const std::string& expression, const std::string& punct)=0;

	/// \brief Map a document to a text string as input of POS tagging
	/// \param[ín] dclass document class of document to markup
	/// \param[in] content input to map
	virtual std::string getPosTaggerInput( const analyzer::DocumentClass& dclass, const std::string& content) const=0;

	/// \brief Create a context for POS 
	/// \param[ín] data collected data from POS tagging output
	virtual PosTaggerContextInterface* createContext( const PosTaggerDataInterface* data) const;
};

}//namespace
#endif

