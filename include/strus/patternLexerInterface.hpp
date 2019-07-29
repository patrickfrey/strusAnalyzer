/*
 * Copyright (c) 2016 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Interface for instantiating the data structure of an automaton for detecting lexems used as basic entities by pattern matching in text
/// \file "patternLexerInterface.hpp"
#ifndef _STRUS_ANALYZER_PATTERN_LEXER_INTERFACE_HPP_INCLUDED
#define _STRUS_ANALYZER_PATTERN_LEXER_INTERFACE_HPP_INCLUDED
#include "strus/structView.hpp"
#include <vector>
#include <string>

namespace strus
{
/// \brief Forward declaration
class PatternLexerInstanceInterface;

/// \brief Interface for instantiating the data structure of an automaton for detecting lexems used as basic entities by pattern matching in text
class PatternLexerInterface
{
public:
	/// \brief Destructor
	virtual ~PatternLexerInterface(){}

	/// \brief Get the list of option names you can pass to PatternLexerInstanceInterface::compile
	/// \return NULL terminated array of strings
	virtual std::vector<std::string> getCompileOptionNames() const=0;

	/// \brief Create an instance to build the regular expressions for a lexem matcher
	/// \return the lexer instance
	virtual PatternLexerInstanceInterface* createInstance() const=0;

	/// \brief Get the name of the function
	/// \return the identifier
	virtual const char* name() const=0;

	/// \brief Return a structure with all definitions for introspection
	/// \return the structure with all definitions for introspection
	virtual StructView view() const=0;
};

} //namespace
#endif

