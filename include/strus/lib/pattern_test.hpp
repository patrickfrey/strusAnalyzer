/*
 * Copyright (c) 2016 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Exported functions of the strus standard pattern matching library for tests
/// \note This implementation of the pattern lexer/matcher interfaces is only intended to use for verifcation in tests and not for productive use
/// \file pattern_test.hpp
#ifndef _STRUS_PATTERN_TEST_LIB_HPP_INCLUDED
#define _STRUS_PATTERN_TEST_LIB_HPP_INCLUDED
#include <cstdio>

/// \brief strus toplevel namespace
namespace strus {

/// \brief Forward declaration
class PatternLexerInterface;
/// \brief Forward declaration
class PatternMatcherInterface;
/// \brief Forward declaration
class TokenMarkupInstanceInterface;
/// \brief Forward declaration
class ErrorBufferInterface;

/// \brief Create the interface for regular expression matching usable as groud truth for testing
PatternLexerInterface* createPatternLexer_test(
		ErrorBufferInterface* errorhnd);

/// \brief Create the interface for pattern matching usable as groud truth for testing
PatternMatcherInterface* createPatternMatcher_test(
		ErrorBufferInterface* errorhnd);

}//namespace
#endif

