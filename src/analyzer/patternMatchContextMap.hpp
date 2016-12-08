/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef _STRUS_ANALYZER_PATTERN_FEATURE_CONTEXT_MAP_HPP_INCLUDED
#define _STRUS_ANALYZER_PATTERN_FEATURE_CONTEXT_MAP_HPP_INCLUDED
#include "strus/patternMatcherContextInterface.hpp"
#include "strus/patternLexerContextInterface.hpp"
#include "strus/patternTermFeederInstanceInterface.hpp"
#include "strus/reference.hpp"
#include "bindTerm.hpp"
#include "patternMatchConfig.hpp"
#include "patternMatchConfigMap.hpp"
#include <map>
#include <vector>
#include <string>
#include <stdexcept>

namespace strus {

struct PreProcPatternMatchContext
{
	typedef Reference<PatternMatcherContextInterface> PatternMatcherContextReference;
	typedef Reference<PatternLexerContextInterface> PatternLexerContextReference;
	typedef std::map<std::size_t,std::size_t> SegPosContentPosMap;

	PreProcPatternMatchContext( const PreProcPatternMatchConfig& config);
	PreProcPatternMatchContext( const PreProcPatternMatchContext& o)
		:m_config(o.m_config)
		,m_matcher(o.m_matcher)
		,m_lexer(o.m_lexer){}

	void process( std::size_t segpos, const char* seg, std::size_t segsize);
	std::vector<BindTerm> fetchResults();

	const PreProcPatternMatchConfig* m_config;
	PatternMatcherContextReference m_matcher;
	PatternLexerContextReference m_lexer;
	std::string m_content;
	SegPosContentPosMap m_segPosContentPosMap;
};

struct PostProcPatternMatchContext
{
	typedef Reference<PatternMatcherContextInterface> PatternMatcherContextReference;

	PostProcPatternMatchContext( const PostProcPatternMatchConfig& config);
	PostProcPatternMatchContext( const PostProcPatternMatchContext& o)
		:m_config(o.m_config)
		,m_matcher(o.m_matcher)
		,m_feeder(o.m_feeder){}

	void process( const std::vector<BindTerm>& input);
	std::vector<BindTerm> fetchResults();

	const PostProcPatternMatchConfig* m_config;
	PatternMatcherContextReference m_matcher;
	const PatternTermFeederInstanceInterface* m_feeder;
	std::vector<BindTerm> m_input;
};



/// \brief Set of pre processing pattern matching programs defined
class PreProcPatternMatchContextMap
{
public:
	PreProcPatternMatchContextMap( const PreProcPatternMatchConfigMap& config);
	PreProcPatternMatchContextMap( const PreProcPatternMatchContextMap& o)
		:m_ar(o.m_ar){}
	~PreProcPatternMatchContextMap(){}

	const PreProcPatternMatchContext& context( std::size_t idx) const
	{
		if (idx <= 0 || idx > m_ar.size()) throw std::logic_error("array bound read (document analyzer pattern match context)");
		return m_ar[ idx-1];
	}
	PreProcPatternMatchContext& context( std::size_t idx)
	{
		if (idx <= 0 || idx > m_ar.size()) throw std::logic_error("array bound write (document analyzer pattern match context)");
		return m_ar[ idx-1];
	}

private:
	std::vector<PreProcPatternMatchContext> m_ar;
};


/// \brief Set of post processing pattern matching programs defined
class PostProcPatternMatchContextMap
{
public:
	PostProcPatternMatchContextMap( const PostProcPatternMatchConfigMap& config);
	PostProcPatternMatchContextMap( const PostProcPatternMatchContextMap& o)
		:m_ar(o.m_ar){}
	~PostProcPatternMatchContextMap(){}

	typedef std::vector<PostProcPatternMatchContext>::iterator iterator;
	iterator begin()		{return m_ar.begin();}
	iterator end()			{return m_ar.end();}

private:
	std::vector<PostProcPatternMatchContext> m_ar;
};

}//namespace
#endif

