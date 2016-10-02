/*
 * Copyright (c) 2016 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Interface for detecting lexems used as basic entities by pattern matching in text
/// \file "patternLexerInterface.hpp"
#ifndef _STRUS_ANALYZER_PATTERN_LEXER_CONTEXT_INTERFACE_HPP_INCLUDED
#define _STRUS_ANALYZER_PATTERN_LEXER_CONTEXT_INTERFACE_HPP_INCLUDED
#include "strus/analyzer/patternLexem.hpp"
#include <vector>

namespace strus
{

/// \brief Interface for detecting lexems used as basic entities by pattern matching in text
class PatternLexerContextInterface
{
public:
	/// \brief Destructor
	virtual ~PatternLexerContextInterface(){}

	/// \brief Do process a document source string to return a list of labeled terms (lexems) found that matched
	/// \param[in] src pointer to source of the tokens to match against
	/// \param[in] srclen length of src to scan in bytes
	/// \return list of matched lexems
	virtual std::vector<analyzer::PatternLexem> match( const char* src, std::size_t srclen)=0;
};

} //namespace
#endif


