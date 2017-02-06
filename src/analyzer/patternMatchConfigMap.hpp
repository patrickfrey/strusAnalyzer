/*
 * Copyright (c) 2016 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef _STRUS_DOCUMENT_ANALYZER_PATTERN_MATCH_CONFIG_MAP_HPP_INCLUDED
#define _STRUS_DOCUMENT_ANALYZER_PATTERN_MATCH_CONFIG_MAP_HPP_INCLUDED
#include "patternMatchConfig.hpp"
#include <vector>
#include <string>

namespace strus
{

/// \brief Set of pre processing pattern matching programs defined
class PreProcPatternMatchConfigMap
{
public:
	PreProcPatternMatchConfigMap()
		:m_ar(){}
	~PreProcPatternMatchConfigMap(){}

	unsigned int definePatternMatcher(
		const std::string& patternTypeName,
		PatternMatcherInstanceInterface* matcher,
		PatternLexerInstanceInterface* lexer,
		bool allowCrossSegmentMatches);

	const PreProcPatternMatchConfig& config( int idx) const;

	typedef std::vector<PreProcPatternMatchConfig>::const_iterator const_iterator;
	const_iterator begin() const		{return m_ar.begin();}
	const_iterator end() const		{return m_ar.end();}

private:
	std::vector<PreProcPatternMatchConfig> m_ar;
};


/// \brief Set of post processing pattern matching programs defined
class PostProcPatternMatchConfigMap
{
public:
	PostProcPatternMatchConfigMap()
		:m_ar(){}
	~PostProcPatternMatchConfigMap(){}

	unsigned int definePatternMatcher(
		const std::string& patternTypeName,
		PatternMatcherInstanceInterface* matcher,
		PatternTermFeederInstanceInterface* feeder,
		bool allowCrossSegmentMatches);

	const PostProcPatternMatchConfig& config( int idx) const;

	typedef std::vector<PostProcPatternMatchConfig>::const_iterator const_iterator;
	const_iterator begin() const		{return m_ar.begin();}
	const_iterator end() const		{return m_ar.end();}

private:
	std::vector<PostProcPatternMatchConfig> m_ar;
};

}//namespace
#endif


