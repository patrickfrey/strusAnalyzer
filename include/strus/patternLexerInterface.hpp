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
	virtual std::vector<std::string> getCompileOptions() const=0;

	/// \brief Create an instance to build the regular expressions for a term matcher
	/// \return the term matcher instance
	virtual PatternLexerInstanceInterface* createInstance() const=0;
};

} //namespace
#endif

