/*
 * Copyright (c) 2016 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Interface for creating an automaton for detecting patterns of tokens in a document stream
/// \file "patternMatcherInterface.hpp"
#ifndef _STRUS_ANALYZER_PATTERN_MATCH_INTERFACE_HPP_INCLUDED
#define _STRUS_ANALYZER_PATTERN_MATCH_INTERFACE_HPP_INCLUDED
#include "strus/structView.hpp"
#include <vector>
#include <string>

namespace strus
{
/// \brief Forward declaration
class PatternMatcherInstanceInterface;

/// \brief Interface for creating an automaton for detecting patterns of tokens in a document stream
class PatternMatcherInterface
{
public:
	/// \brief Destructor
	virtual ~PatternMatcherInterface(){}

	/// \brief Get the list of option names you can pass to PatternMatcherInstanceInterface::compile
	/// \return NULL terminated array of strings
	virtual std::vector<std::string> getCompileOptionNames() const=0;

	/// \brief Create an instance to build the rules of a pattern matcher
	/// \return the pattern matcher instance
	virtual PatternMatcherInstanceInterface* createInstance() const=0;

	/// \brief Get the name of the function
	/// \return the identifier
	virtual const char* name() const=0;

	/// \brief Return a structure with all definitions for introspection
	/// \return the structure with all definitions for introspection
	virtual StructView view() const=0;
};

} //namespace
#endif

