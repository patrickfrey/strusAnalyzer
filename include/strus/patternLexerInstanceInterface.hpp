/*
 * Copyright (c) 2016 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Interface for building the automaton for detecting lexems used as basic entities by pattern matching in text
/// \file "patternLexerInstanceInterface.hpp"
#ifndef _STRUS_ANALYZER_PATTERN_LEXER_INSTANCE_INTERFACE_HPP_INCLUDED
#define _STRUS_ANALYZER_PATTERN_LEXER_INSTANCE_INTERFACE_HPP_INCLUDED
#include "strus/analyzer/positionBind.hpp"
#include <string>

namespace strus
{

/// \brief Forward declaration
class PatternLexerContextInterface;

/// \brief Interface for building the automaton for detecting lexems used as basic entities by pattern matching in text
class PatternLexerInstanceInterface
{
public:
	/// \brief Destructor
	virtual ~PatternLexerInstanceInterface(){}

	/// \brief Define an option value for the compilation
	/// \param[in] name option name
	/// \param[in] value option value
	virtual void defineOption( const std::string& name, double value)=0;

	/// \brief Define a pattern for detecting a basic lexem of this pattern matching lexer
	/// \param[in] id identifier given to the lexem, 0 if the lexem is not part of the output (only used for assigning ordinal positions).
	/// \param[in] expression expression string defining the lexem
	/// \param[in] resultIndex index of subexpression that defines the result lexem, 0 for the whole match
	/// \param[in] level weight of this lexical pattern. A lexical pattern match causes the suppressing of all lexems of lower level that are completely covered by one lexem of this pattern
	/// \param[in] posbind defines how the ordinal position is assigned to the result lexem
	/// \remark For performance it may be significantly better to define rules without resultIndex selection. But sometimes you need pattern sub expression selections for defining the result.
	virtual void defineLexem(
			unsigned int id,
			const std::string& expression,
			unsigned int resultIndex,
			unsigned int level,
			analyzer::PositionBind posbind)=0;

	/// \brief Define a symbol, an instance of a basic lexem, that gets a different id than the basic lexem
	/// \param[in] id identifier given to this symbol
	/// \param[in] lexemid identifier of the basic lexem (defined with defineLexem) this symbol belongs to
	/// \param[in] name name (value string) of the symbol
	/// \note The idea of symbols is to keep the automaton for lexical pattern detection small and detect symbols as combination of a lexical pattern match plus an ordinary dictionary lookup
	virtual void defineSymbol(
			unsigned int id,
			unsigned int lexemid,
			const std::string& name)=0;

	/// \brief Get the value of a defined symbol
	/// \param[in] lexemid identifier of the basic lexem this symbol belongs to
	/// \param[in] name name (value string) of the symbol
	/// \return the symbol identifier or 0, if not defined
	/// \remark this function is needed because symbols are most likely implicitely defined on demand by reference
	virtual unsigned int getSymbol(
			unsigned int lexemid,
			const std::string& name) const=0;

	/// \brief Compile all patterns and symbols defined
	/// \return true on success, false on error (error reported in error buffer)
	/// \remark This function has to be called in order to make the patterns active, resp. before calling 'createContext()'
	virtual bool compile()=0;

	/// \brief Create the context to process a chunk of text with this text matcher
	/// \return the lexer context
	/// \remark The context cannot be reset. So the context has to be recreated for every processed unit (document)
	virtual PatternLexerContextInterface* createContext() const=0;
};

} //namespace
#endif

