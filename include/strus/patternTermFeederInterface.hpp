/*
 * Copyright (c) 2016 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Interface for defining a mapping of terms of the document analysis outout as lexems used as basic entities by pattern matching
/// \file "patternTermFeederInterface.hpp"
#ifndef _STRUS_ANALYZER_PATTERN_TERM_FEEDER_INTERFACE_HPP_INCLUDED
#define _STRUS_ANALYZER_PATTERN_TERM_FEEDER_INTERFACE_HPP_INCLUDED
#include "strus/structView.hpp"
#include <vector>
#include <string>

namespace strus
{
/// \brief Forward declaration
class PatternTermFeederInstanceInterface;

/// \brief Interface for instantiating the data structure of an automaton for detecting lexems used as basic entities by pattern matching in text
class PatternTermFeederInterface
{
public:
	/// \brief Destructor
	virtual ~PatternTermFeederInterface(){}

	/// \brief Create an instance to define the mappings of terms to pattern lexems and a method to call the mappings defined.
	/// \return the term feeder instance
	virtual PatternTermFeederInstanceInterface* createInstance() const=0;

	/// \brief Get the name of the function
	/// \return the identifier
	virtual const char* name() const=0;

	/// \brief Return a structure with all definitions for introspection
	/// \return the structure with all definitions for introspection
	virtual StructView view() const=0;
};

} //namespace
#endif

