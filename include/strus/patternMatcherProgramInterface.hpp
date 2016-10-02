/*
 * Copyright (c) 2016 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Interface to load pattern definitions from source
/// \file "patternMatcherProgramInterface.hpp"
#ifndef _STRUS_ANALYZER_PATTERN_MATCHER_PROGRAM_INTERFACE_HPP_INCLUDED
#define _STRUS_ANALYZER_PATTERN_MATCHER_PROGRAM_INTERFACE_HPP_INCLUDED

namespace strus {

/// \brief Forward declaration
class PatternMatcherProgramInstanceInterface;

/// \brief StrusStream interface to load pattern definitions from source
class PatternMatcherProgramInterface
{
public:
	/// \brief Destructor
	virtual ~PatternMatcherProgramInterface(){}

	/// \brief Create an instance to load the rules of a pattern matcher from source
	/// \return the pattern matcher program instance
	virtual PatternMatcherProgramInstanceInterface* createInstance() const=0;
};

} //namespace
#endif

