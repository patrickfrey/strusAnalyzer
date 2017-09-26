/*
 * Copyright (c) 2016 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "patternMatchConfigMap.hpp"
#include "private/internationalization.hpp"
#include "private/utils.hpp"
#include "featureConfigMap.hpp"

using namespace strus;

const PreProcPatternMatchConfig& PreProcPatternMatchConfigMap::config( int idx) const
{
	if (idx <= 0 || (std::size_t)idx > m_ar.size())
	{
		throw strus::runtime_error( "%s", _TXT("internal: unknown index of feature"));
	}
	return m_ar[ idx-1];
}

unsigned int PreProcPatternMatchConfigMap::definePatternMatcher(
		const std::string& patternTypeName,
		PatternMatcherInstanceInterface* matcher,
		PatternLexerInstanceInterface* lexer,
		bool allowCrossSegmentMatches)
{
	try
	{
		if (m_ar.size()+1 >= MaxNofPatternMatchSegments)
		{
			throw strus::runtime_error( "%s", _TXT("number of features defined exceeds maximum limit"));
		}
		m_ar.reserve( m_ar.size()+1);
		m_ar.push_back( PreProcPatternMatchConfig( utils::tolower(patternTypeName), matcher, lexer, allowCrossSegmentMatches));
		return m_ar.size();
	}
	catch (const std::bad_alloc&)
	{
		delete matcher;
		delete lexer;
		throw strus::runtime_error( "%s", _TXT("memory allocation error defining pattern matcher"));
	}
	catch (const std::runtime_error& err)
	{
		delete matcher;
		delete lexer;
		throw strus::runtime_error( _TXT("error defining pattern matcher: '%s'"), err.what());
	}
}



const PostProcPatternMatchConfig& PostProcPatternMatchConfigMap::config( int idx) const
{
	if (idx <= 0 || (std::size_t)idx > m_ar.size())
	{
		throw strus::runtime_error( "%s", _TXT("internal: unknown index of feature"));
	}
	return m_ar[ idx-1];
}

unsigned int PostProcPatternMatchConfigMap::definePatternMatcher(
		const std::string& patternTypeName,
		PatternMatcherInstanceInterface* matcher,
		PatternTermFeederInstanceInterface* feeder,
		bool allowCrossSegmentMatches)
{
	try
	{
		if (m_ar.size()+1 >= MaxNofPatternMatchSegments)
		{
			throw strus::runtime_error( "%s", _TXT("number of features defined exceeds maximum limit"));
		}
		m_ar.reserve( m_ar.size()+1);
		m_ar.push_back( PostProcPatternMatchConfig( utils::tolower(patternTypeName), matcher, feeder, allowCrossSegmentMatches));
		return m_ar.size();
	}
	catch (const std::bad_alloc&)
	{
		delete matcher;
		delete feeder;
		throw strus::runtime_error( "%s", _TXT("memory allocation error defining pattern matcher"));
	}
	catch (const std::runtime_error& err)
	{
		delete matcher;
		delete feeder;
		throw strus::runtime_error( _TXT("error defining pattern matcher: '%s'"), err.what());
	}
}



