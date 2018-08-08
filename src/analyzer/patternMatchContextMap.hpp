/*
 * Copyright (c) 2016 Patrick P. Frey
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
#include "strus/segmenterContextInterface.hpp"
#include "strus/reference.hpp"
#include "bindTerm.hpp"
#include "patternMatchConfig.hpp"
#include "patternMatchConfigMap.hpp"
#include <map>
#include <vector>
#include <string>
#include <stdexcept>

namespace strus {

/// \brief Forward declaration
class ErrorBufferInterface;
/// \brief Forward declaration
class DebugTraceContextInterface;

struct PreProcPatternMatchContext
{
	typedef Reference<PatternMatcherContextInterface> PatternMatcherContextReference;
	typedef Reference<PatternLexerContextInterface> PatternLexerContextReference;
	typedef std::map<SegmenterPosition,std::size_t> SegPosContentPosMap;

	PreProcPatternMatchContext( const PreProcPatternMatchConfig& config, ErrorBufferInterface* errorhnd_);
	~PreProcPatternMatchContext();

	void process( const SegmenterPosition& segpos, const char* seg, std::size_t segsize);
	std::vector<BindTerm> fetchResults();
	void clear();

	const PatternMatcherContextInterface* matcher() const		{return m_matcher.get();}
	const PatternLexerContextInterface* lexer() const		{return m_lexer.get();}

private:
	std::string formatResult( const char* value) const;
	PreProcPatternMatchContext( const PreProcPatternMatchContext&) {}//... non copyable

private:
	const PreProcPatternMatchConfig* m_config;
	PatternMatcherContextReference m_matcher;
	PatternLexerContextReference m_lexer;
	std::string m_content;
	int m_ordposofs;
	SegPosContentPosMap m_segPosContentPosMap;
	ErrorBufferInterface* m_errorhnd;
	DebugTraceContextInterface* m_debugtrace;
};

struct PostProcPatternMatchContext
{
	typedef Reference<PatternMatcherContextInterface> PatternMatcherContextReference;

	PostProcPatternMatchContext( const PostProcPatternMatchConfig& config, ErrorBufferInterface* errorhnd_);
	~PostProcPatternMatchContext();

	void process( const std::vector<BindTerm>& input);
	std::vector<BindTerm> fetchResults();
	void clear();

	const PatternMatcherContextInterface* matcher() const		{return m_matcher.get();}
	const PatternTermFeederInstanceInterface* feeder() const	{return m_feeder;}

private:
	std::string formatResult( const char* value) const;
	PostProcPatternMatchContext( const PostProcPatternMatchContext&) {}//... non copyable

private:
	const PostProcPatternMatchConfig* m_config;
	PatternMatcherContextReference m_matcher;
	const PatternTermFeederInstanceInterface* m_feeder;
	std::vector<BindLexem> m_input;
	bool m_fetchResults_called;
	ErrorBufferInterface* m_errorhnd;
	DebugTraceContextInterface* m_debugtrace;
};


typedef strus::Reference<PreProcPatternMatchContext> PreProcPatternMatchContextReference;

/// \brief Set of pre processing pattern matching programs defined
class PreProcPatternMatchContextMap
{
public:
	PreProcPatternMatchContextMap( const PreProcPatternMatchConfigMap& config, ErrorBufferInterface* errorhnd_);
	PreProcPatternMatchContextMap( const PreProcPatternMatchContextMap& o)
		:m_ar(o.m_ar){}
	~PreProcPatternMatchContextMap();

	const PreProcPatternMatchContext& context( std::size_t idx) const
	{
		if (idx <= 0 || idx > m_ar.size()) throw std::logic_error("array bound read (document analyzer pattern match context)");
		return *m_ar[ idx-1].get();
	}
	PreProcPatternMatchContext& context( std::size_t idx)
	{
		if (idx <= 0 || idx > m_ar.size()) throw std::logic_error("array bound write (document analyzer pattern match context)");
		return *m_ar[ idx-1].get();
	}

	typedef std::vector<PreProcPatternMatchContextReference>::iterator iterator;
	iterator begin()		{return m_ar.begin();}
	iterator end()			{return m_ar.end();}

	void clear();

private:
	std::vector<PreProcPatternMatchContextReference> m_ar;
	ErrorBufferInterface* m_errorhnd;
	DebugTraceContextInterface* m_debugtrace;
};

typedef strus::Reference<PostProcPatternMatchContext> PostProcPatternMatchContextReference;

/// \brief Set of post processing pattern matching programs defined
class PostProcPatternMatchContextMap
{
public:
	PostProcPatternMatchContextMap( const PostProcPatternMatchConfigMap& config, ErrorBufferInterface* errorhnd_);
	PostProcPatternMatchContextMap( const PostProcPatternMatchContextMap& o)
		:m_ar(o.m_ar){}
	~PostProcPatternMatchContextMap();

	typedef std::vector<PostProcPatternMatchContextReference>::iterator iterator;
	iterator begin()		{return m_ar.begin();}
	iterator end()			{return m_ar.end();}

	void clear();

private:
	std::vector<PostProcPatternMatchContextReference> m_ar;
	ErrorBufferInterface* m_errorhnd;
	DebugTraceContextInterface* m_debugtrace;
};

}//namespace
#endif

