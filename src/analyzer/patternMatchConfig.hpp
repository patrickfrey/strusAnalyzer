/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef _STRUS_ANALYZER_PATTERN_MATCH_CONFIG_HPP_INCLUDED
#define _STRUS_ANALYZER_PATTERN_MATCH_CONFIG_HPP_INCLUDED
#include "strus/patternMatcherInstanceInterface.hpp"
#include "strus/patternTermFeederInstanceInterface.hpp"
#include "strus/patternLexerInstanceInterface.hpp"
#include "strus/analyzer/featureOptions.hpp"
#include "strus/reference.hpp"
#include <string>

namespace strus
{
/// \brief Forward declaration
class ErrorBufferInterface;

class PreProcPatternMatchConfig
{
public:
	PreProcPatternMatchConfig(
			const std::string& patternTypeName_,
			PatternMatcherInstanceInterface* matcher_,
			PatternLexerInstanceInterface* lexer_)
		:m_patternTypeName(patternTypeName_),m_matcher(),m_lexer()
	{
		try
		{
			m_matcher.reset( matcher_);
			m_lexer.reset( lexer_);
		}
		catch (const std::bad_alloc&)
		{
			m_matcher.release();
			m_lexer.release();
		}
	}

	PreProcPatternMatchConfig( const PreProcPatternMatchConfig& o)
		:m_patternTypeName(o.m_patternTypeName)
		,m_matcher(o.m_matcher)
		,m_lexer(o.m_lexer){}

	const std::string& patternTypeName() const			{return m_patternTypeName;}
	const PatternMatcherInstanceInterface* matcher() const		{return m_matcher.get();}
	const PatternLexerInstanceInterface* lexer() const		{return m_lexer.get();}

private:
	std::string m_patternTypeName;
	Reference<PatternMatcherInstanceInterface> m_matcher;
	Reference<PatternLexerInstanceInterface> m_lexer;
};


class PostProcPatternMatchConfig
{
public:
	PostProcPatternMatchConfig(
			const std::string& patternTypeName_,
			PatternMatcherInstanceInterface* matcher_,
			PatternTermFeederInstanceInterface* feeder_)
		:m_patternTypeName(patternTypeName_),m_matcher(),m_feeder()
	{
		try
		{
			m_matcher.reset( matcher_);
			m_feeder.reset( feeder_);
		}
		catch (const std::bad_alloc&)
		{
			m_matcher.release();
			m_feeder.release();
		}
	}

	PostProcPatternMatchConfig( const PostProcPatternMatchConfig& o)
		:m_patternTypeName(o.m_patternTypeName)
		,m_matcher(o.m_matcher)
		,m_feeder(o.m_feeder){}

	const std::string& patternTypeName() const			{return m_patternTypeName;}
	const PatternMatcherInstanceInterface* matcher() const		{return m_matcher.get();}
	const PatternTermFeederInstanceInterface* feeder() const	{return m_feeder.get();}

private:
	std::string m_patternTypeName;
	Reference<PatternMatcherInstanceInterface> m_matcher;
	Reference<PatternTermFeederInstanceInterface> m_feeder;
};

} //namespace
#endif

