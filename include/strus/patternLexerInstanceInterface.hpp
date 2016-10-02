/*
 * Copyright (c) 2016 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Interface for building the automaton for detecting tokens defined as expressions in text
/// \file "charRegexMatchInstanceInterface.hpp"
#ifndef _STRUS_ANALYZER_PATTERN_LEXER_INSTANCE_INTERFACE_HPP_INCLUDED
#define _STRUS_ANALYZER_PATTERN_LEXER_INSTANCE_INTERFACE_HPP_INCLUDED
#include "strus/analyzer/patternLexerOptions.hpp"
#include "strus/analyzer/positionBind.hpp"
#include <string>

namespace strus
{

/// \brief Forward declaration
class PatternLexerContextInterface;

/// \brief Interface for building the automaton for detecting tokens defined as regular expressions in text
class PatternLexerInstanceInterface
{
public:
	/// \brief Destructor
	virtual ~PatternLexerInstanceInterface(){}

	/// \brief Define a pattern for this pattern matching lexer
	/// \param[in] id identifier given to the lexem, 0 if the lexem is not part of the output (only used for assigning ordinal positions).
	/// \param[in] expression expression string defining the lexem
	/// \param[in] resultIndex index of subexpression that defines the result lexem, 0 for the whole match
	/// \param[in] level weight of the lexical pattern. A pattern causes the suppressing of all lexems of lower level that are completely covered by one lexem of this pattern
	/// \param[in] posbind defines how the ordinal position is assigned to the result lexem
	/// \remark For performance it may be significantly better to define rules without resultIndex selection.
	virtual void definePattern(
			unsigned int id,
			const std::string& expression,
			unsigned int resultIndex,
			unsigned int level,
			analyzer::PositionBind posbind)=0;

	/// \brief Define a symbol, an instance of a pattern, that gets a different id than the pattern
	/// \param[in] id identifier given to the result substring, 0 if the result term is not appearing in the output
	/// \param[in] patternid identifier of the pattern this symbol belongs to
	/// \param[in] name name (value string) of the symbol
	virtual void defineSymbol(
			unsigned int id,
			unsigned int patternid,
			const std::string& name)=0;

	/// \brief Get the value of a defined symbol
	/// \param[in] patternid identifier of the pattern this symbol belongs to
	/// \param[in] name name (value string) of the symbol
	/// \return the symbol identifier or 0, if not defined
	/// \remark this function is needed because symbols are most likely implicitely defined on demand by reference
	virtual unsigned int getSymbol(
			unsigned int patternid,
			const std::string& name) const=0;

	/// \brief Compile all patterns and symbols defined
	/// \return true on success, false on error (error reported in error buffer)
	/// \remark This function has to be called in order to make the patterns active, resp. before calling 'createContext()'
	virtual bool compile( const analyzer::PatternLexerOptions& opts)=0;

	/// \brief Create the context to process a chunk of text with this text matcher
	/// \return the term matcher context
	/// \remark The context cannot be reset. So the context has to be recreated for every processed unit (document)
	virtual PatternLexerContextInterface* createContext() const=0;
};

} //namespace
#endif

