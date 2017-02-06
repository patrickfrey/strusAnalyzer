/*
 * Copyright (c) 2016 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Structure with options for optimization of a token pattern match automaton
/// \file "patternMatcherOptions.hpp"
#ifndef _STRUS_ANALYZER_PATTERN_MATCHER_OPTIONS_HPP_INCLUDED
#define _STRUS_ANALYZER_PATTERN_MATCHER_OPTIONS_HPP_INCLUDED
#include <vector>
#include <string>

namespace strus {
namespace analyzer {

/// \brief Structure with options for optimization of a token pattern match automaton
class PatternMatcherOptions
{
public:
	///\brief Default constructor
	PatternMatcherOptions(){}
	///\brief Copy constructor
	PatternMatcherOptions( const PatternMatcherOptions& o)
		:m_opts(o.m_opts){}

	/// \brief Add an option definition
	/// \note The standard token pattern match option definitions are:
	///		"stopwordOccurrenceFactor"	: what fraction of the whole collection a word must occurr (df) to be considered a stopword
	///		"weightFactor"			: what weight factor an alternative event must have to be considered as alernative key event triggering the detection of the rule
	///		"maxRange"			: maximum proximity range a rule can reach before not considered anymore for a rule rewriting (alternative key event)
	///		"exclusive"			: pattern match result completely covered by other patterns are suppressed
	PatternMatcherOptions& operator()( const std::string& opt, double value)
	{
		m_opts.push_back( OptionDef( opt, value));
		return *this;
	}

	typedef std::pair<std::string,double> OptionDef;
	typedef std::vector<OptionDef>::const_iterator const_iterator;

	/// \brief Get iterator at first element of the option list
	const_iterator begin() const	{return m_opts.begin();}
	/// \brief Get iterator marking the end of the option list
	const_iterator end() const	{return m_opts.end();}

	std::size_t size() const	{return m_opts.size();}

private:
	std::vector<OptionDef> m_opts;	///< list of option definitions
};

}} //namespace
#endif

