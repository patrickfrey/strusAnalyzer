/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef _STRUS_ANALYZER_SEGMENT_PROCESSOR_HPP_INCLUDED
#define _STRUS_ANALYZER_SEGMENT_PROCESSOR_HPP_INCLUDED
#include "featureConfigMap.hpp"
#include "patternFeatureConfigMap.hpp"
#include "searchIndexStructure.hpp"
#include "bindTerm.hpp"
#include "strus/analyzer/document.hpp"
#include "strus/analyzer/queryTermExpression.hpp"
#include "strus/analyzer/positionBind.hpp"
#include "strus/analyzer/token.hpp"
#include <vector>
#include <string>
#include <map>

namespace strus
{
/// \brief Forward declaration
class ErrorBufferInterface;
/// \brief Forward declaration
class DebugTraceContextInterface;

struct SegPosDef
{
	int start_strpos;
	int end_strpos;
	int segpos;

	SegPosDef( int start_strpos_, int end_strpos_, int segpos_)
		:start_strpos(start_strpos_),end_strpos(end_strpos_),segpos(segpos_){}
#if __cplusplus >= 201103L
	SegPosDef( SegPosDef&& ) = default;
	SegPosDef( const SegPosDef& ) = default;
	SegPosDef& operator= ( SegPosDef&& ) = default;
	SegPosDef& operator= ( const SegPosDef& ) = default;
#else
	SegPosDef( const SegPosDef& o)
		:start_strpos(o.start_strpos),end_strpos(o.end_strpos),segpos(o.segpos){}
#endif
};

class SegmentProcessor
{
public:
	SegmentProcessor(
			const FeatureConfigMap& featureConfigMap_,
			const PatternFeatureConfigMap& patternFeatureConfigMap_,
			ErrorBufferInterface* errorhnd_);
	~SegmentProcessor();

	void clearTermMaps();
	void processDocumentSegment(
			int featidx,
			std::size_t segmentpos,
			const char* segmentptr,
			std::size_t segmentsize);
	void concatDocumentSegment(
			int featidx,
			std::size_t segmentpos,
			const char* segmentptr,
			std::size_t segmentsize);

	void processConcatenated();
	void eliminateCovered();

	void processPatternMatchResult( const std::vector<BindTerm>& result);

	/// \brief Fetch the currently processed document
	analyzer::Document fetchDocument(
		const std::vector<SeachIndexStructureConfig>& structureConfigs,
		const std::vector<SearchIndexStructure>& structures);

	class QueryElement
		:public analyzer::QueryTerm
	{
	public:
		QueryElement( int fieldno_, int pos_, int priority_, const analyzer::QueryTerm& term_)
			:analyzer::QueryTerm(term_),m_fieldno(fieldno_),m_priority(priority_),m_pos(pos_){}
#if __cplusplus >= 201103L
		QueryElement( QueryElement&& ) = default;
		QueryElement( const QueryElement& ) = default;
		QueryElement& operator= ( QueryElement&& ) = default;
		QueryElement& operator= ( const QueryElement& ) = default;
#else
		QueryElement( const QueryElement& o)
			:analyzer::QueryTerm(o),m_fieldno(o.m_fieldno),m_priority(o.m_priority),m_pos(o.m_pos){}
#endif

		int fieldno() const		{return m_fieldno;}
		int priority() const		{return m_priority;}
		int pos() const			{return m_pos;}
		int endpos() const		{return m_pos+len();}

		static bool orderPosition( const QueryElement& a, const QueryElement& b)
		{
			// Order in a way that an element can only be covered by an element with a higher priority
			// if this element is appearing before it. So we have only to look to predecessor elements
			// when calculating if an element is ousted by a covering element with higher prio.
			if (a.m_fieldno == b.m_fieldno)
			{
				if (a.m_pos == b.m_pos)
				{
					if (a.m_priority == b.m_priority)
					{
						return (a.len() > b.len());
					}
					else return (a.m_priority > b.m_priority);
				}
				else return (a.m_pos < b.m_pos);
			}
			else return (a.m_fieldno < b.m_fieldno);
		}

	private:
		int m_fieldno;
		int m_priority;
		int m_pos;
	};

	/// \brief Fetch the currently processed query
	/// \return the query object without grouping
	std::vector<QueryElement> fetchQuery() const;

	const std::vector<BindTerm>& searchTerms() const	{return m_searchTerms;}
	const std::vector<BindTerm>& forwardTerms() const	{return m_forwardTerms;}
	const std::vector<BindTerm>& patternLexemTerms() const	{return m_patternLexemTerms;}

private:
	void processDocumentSegment( int featidx, std::size_t segmentpos, const char* elem, std::size_t elemsize, const std::vector<SegPosDef>& concatposmap);
	void processContentTokens( std::vector<BindTerm>& result, const FeatureConfig& feat, const std::vector<analyzer::Token>& tokens, const char* segsrc, std::size_t segmentpos, const std::vector<SegPosDef>& concatposmap) const;

private:
	struct Chunk
	{
		Chunk(){}
		Chunk( const std::string& content_, std::size_t segpos)
			:content(content_),concatposmap()
		{
			concatposmap.push_back( SegPosDef( 0, content.size(), segpos));
		}
#if __cplusplus >= 201103L
		Chunk( Chunk&& ) = default;
		Chunk( const Chunk& ) = default;
		Chunk& operator= ( Chunk&& ) = default;
		Chunk& operator= ( const Chunk& ) = default;
#else
		Chunk( const Chunk& o)
			:content(o.content),concatposmap(o.concatposmap){}
#endif
		std::string content;
		std::vector<SegPosDef> concatposmap;
	};

	typedef std::map<int,Chunk> ConcatenatedMap;

private:
	SegmentProcessor( const SegmentProcessor&){}	//... non copyable
	void operator=( const SegmentProcessor&){}	//... non copyable

private:
	const FeatureConfigMap* m_featureConfigMap;
	const PatternFeatureConfigMap* m_patternFeatureConfigMap;
	ConcatenatedMap m_concatenatedMap;
	std::vector<BindTerm> m_searchTerms;
	std::vector<BindTerm> m_forwardTerms;
	std::vector<BindTerm> m_metadataTerms;
	std::vector<BindTerm> m_attributeTerms;
	std::vector<BindTerm> m_patternLexemTerms;
	ErrorBufferInterface* m_errorhnd;
	DebugTraceContextInterface* m_debugtrace;
};

}//namespace
#endif

