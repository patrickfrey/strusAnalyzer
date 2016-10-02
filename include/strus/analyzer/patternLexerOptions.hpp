/*
 * Copyright (c) 2016 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Structure for options descibing the behaviour and some settings of the character regular expression automaton
/// \file "patternLexerOptions.hpp"
#ifndef _STRUS_ANALYZER_PATTERN_LEXER_OPTIONS_HPP_INCLUDED
#define _STRUS_ANALYZER_PATTERN_LEXER_OPTIONS_HPP_INCLUDED
#include <vector>
#include <string>

namespace strus {
namespace analyzer {

/// \brief Options to stear the lexer automaton
/// \remark Available options depend on implementation
/// \note For the hyperscan backend, the following options are available:
///		"CASELESS", "DOTALL", "MULTILINE", "ALLOWEMPTY", "UCP"
/// \note The options HS_FLAG_UTF8 and HS_FLAG_SOM_LEFTMOST are set implicitely always
class PatternLexerOptions
{
public:
	/// \brief Constructor
	PatternLexerOptions(){}
	/// \brief Copy constructor
	PatternLexerOptions( const PatternLexerOptions& o)
		:m_opts(o.m_opts){}

	/// \brief Add an option definition
	PatternLexerOptions& operator()( const std::string& opt)
	{
		m_opts.push_back( opt);
		return *this;
	}

	typedef std::vector<std::string>::const_iterator const_iterator;
	/// \brief Get iterator at first element of the option list
	const_iterator begin() const	{return m_opts.begin();}
	/// \brief Get iterator marking the end of the option list
	const_iterator end() const	{return m_opts.end();}

private:
	std::vector<std::string> m_opts;	///< list of option strings
};

}} //namespace
#endif

