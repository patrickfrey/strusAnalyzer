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

struct SegPosDef
{
	std::size_t start_strpos;
	std::size_t end_strpos;
	std::size_t segpos;

	SegPosDef( std::size_t start_strpos_, std::size_t end_strpos_, std::size_t segpos_)
		:start_strpos(start_strpos_),end_strpos(end_strpos_),segpos(segpos_){}
	SegPosDef( const SegPosDef& o)
		:start_strpos(o.start_strpos),end_strpos(o.end_strpos),segpos(o.segpos){}
};

class SegmentProcessor
{
public:
	SegmentProcessor(
			const FeatureConfigMap& featureConfigMap_,
			const PatternFeatureConfigMap& patternFeatureConfigMap_)
		:m_featureConfigMap(&featureConfigMap_)
		,m_patternFeatureConfigMap(&patternFeatureConfigMap_)
		,m_concatenatedMap(){}
	~SegmentProcessor(){}

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
	void processPatternMatchResult( const std::vector<BindTerm>& result);

	/// \brief Fetch the currently processed document
	analyzer::Document fetchDocument();

	class QueryElement
		:public analyzer::QueryTerm
	{
	public:
		QueryElement( const QueryElement& o)
			:analyzer::QueryTerm(o),m_fieldno(o.m_fieldno),m_priority(o.m_priority),m_pos(o.m_pos){}
		QueryElement( int fieldno_, int pos_, const analyzer::QueryTerm& term_)
			:analyzer::QueryTerm(term_),m_fieldno(fieldno_),m_priority(0),m_pos(pos_){}

		int fieldno() const		{return m_fieldno;}
		int priority() const		{return m_priority;}
		int pos() const			{return m_pos;}
		int endpos() const		{return m_pos+len();}

		void setPriority( int p)	{m_priority = p;}

		static bool orderPosition( const QueryElement& a, const QueryElement& b)
		{
			if (a.m_fieldno == b.m_fieldno)
			{
				if (a.m_pos == b.m_pos)
				{
					return (a.len() > b.len());
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
		Chunk( const Chunk& o)
			:content(o.content),concatposmap(o.concatposmap){}
	
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
};

}//namespace
#endif

