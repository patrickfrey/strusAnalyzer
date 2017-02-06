/*
 * Copyright (c) 2016 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Interface for detecting patterns (structures formed by atomic lexems) in one document
/// \file "patternMatcherContextInterface.hpp"
#ifndef _STRUS_ANALYZER_PATTERN_MATCHER_CONTEXT_INTERFACE_HPP_INCLUDED
#define _STRUS_ANALYZER_PATTERN_MATCHER_CONTEXT_INTERFACE_HPP_INCLUDED
#include "strus/analyzer/patternLexem.hpp"
#include "strus/analyzer/patternMatcherResult.hpp"
#include "strus/analyzer/patternMatcherStatistics.hpp"
#include <vector>

namespace strus
{

/// \brief Interface for detecting patterns (structures formed by atomic tokens) in one document
class PatternMatcherContextInterface
{
public:
	/// \brief Destructor
	virtual ~PatternMatcherContextInterface(){}

	/// \brief Feed the next input token
	/// \param[in] token the token to feed
	/// \remark The input terms must be fed in ascending order of 'ordpos'
	virtual void putInput( const analyzer::PatternLexem& token)=0;

	/// \brief Get the list of matches detected in the current document
	/// \return the list of matches
	virtual std::vector<analyzer::PatternMatcherResult> fetchResults() const=0;

	/// \brief Get the statistics for global analysis
	/// \return the statistics data gathered during processing
	virtual analyzer::PatternMatcherStatistics getStatistics() const=0;

	/// \brief Reset the pattern matching context for a reuse within another document or query
	virtual void reset()=0;
};

} //namespace
#endif

