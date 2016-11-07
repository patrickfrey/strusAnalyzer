/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef _STRUS_ANALYZER_SEGMENT_PROCESSOR_HPP_INCLUDED
#define _STRUS_ANALYZER_SEGMENT_PROCESSOR_HPP_INCLUDED
#include "featureContextMap.hpp"
#include "featureContextMap.hpp"
#include "strus/analyzer/document.hpp"
#include "strus/analyzer/query.hpp"
#include "strus/analyzer/positionBind.hpp"
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

class BindTerm
{
public:
	BindTerm( const BindTerm& o)
		:m_seg(o.m_seg),m_ofs(o.m_ofs),m_type(o.m_type),m_value(o.m_value),m_posbind(o.m_posbind){}
	BindTerm( unsigned int seg_, unsigned int ofs_, const std::string& type_, const std::string& value_, analyzer::PositionBind posbind_)
		:m_seg(seg_),m_ofs(ofs_),m_type(type_),m_value(value_),m_posbind(posbind_){}

	unsigned int seg() const			{return m_seg;}
	unsigned int ofs() const			{return m_ofs;}
	const std::string& type() const			{return m_type;}
	const std::string& value() const		{return m_value;}
	analyzer::PositionBind posbind() const		{return m_posbind;}

private:
	unsigned int m_seg;
	unsigned int m_ofs;
	std::string m_type;
	std::string m_value;
	analyzer::PositionBind m_posbind;
};


class SegmentProcessor
{
public:
	explicit SegmentProcessor( const FeatureConfigMap& featureConfigMap)
		:m_featureContextMap(featureConfigMap){}

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

	/// \brief Fetch the currently processed document
	analyzer::Document fetchDocument( const analyzer::Document& prevdoc);

	/// \brief Fetch the currently processed query
	/// \return the query object without grouping
	analyzer::Query fetchQuery();

private:
	void processDocumentSegment( int featidx, std::size_t segmentpos, const char* elem, std::size_t elemsize, const std::vector<SegPosDef>& concatposmap);
	void processContentTokens( std::vector<BindTerm>& result, FeatureContext& feat, const std::vector<analyzer::Token>& tokens, const char* segsrc, std::size_t segmentpos, const std::vector<SegPosDef>& concatposmap) const;

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
	FeatureContextMap m_featureContextMap;
	ConcatenatedMap m_concatenatedMap;
	std::vector<BindTerm> m_searchTerms;
	std::vector<BindTerm> m_forwardTerms;
	std::vector<BindTerm> m_metadataTerms;
	std::vector<BindTerm> m_attributeTerms;
};

}//namespace
#endif

