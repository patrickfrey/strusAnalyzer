/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "segmentProcessor.hpp"
#include "private/errorUtils.hpp"
#include "private/internationalization.hpp"
#include "private/debugTraceHelpers.hpp"
#include "strus/segmenterInstanceInterface.hpp"
#include "strus/segmenterContextInterface.hpp"
#include "strus/errorBufferInterface.hpp"
#include "strus/debugTraceInterface.hpp"
#include "strus/analyzer/position.hpp"
#include <set>
#include <iostream>
#include <algorithm>

using namespace strus;

#define STRUS_DBGTRACE_COMPONENT_NAME "analyzer"
#define DEBUG_OPEN( NAME) if (m_debugtrace) m_debugtrace->open( NAME);
#define DEBUG_CLOSE() if (m_debugtrace) m_debugtrace->close();
#define DEBUG_EVENT1( NAME, FMT, X1)				if (m_debugtrace) m_debugtrace->event( NAME, FMT, X1);
#define DEBUG_EVENT4( NAME, FMT, X1, X2, X3, X4)		if (m_debugtrace) m_debugtrace->event( NAME, FMT, X1, X2, X3, X4);
#define DEBUG_EVENT5( NAME, FMT, X1, X2, X3, X4, X5)		if (m_debugtrace) m_debugtrace->event( NAME, FMT, X1, X2, X3, X4, X5);
#define DEBUG_EVENT6( NAME, FMT, X1, X2, X3, X4, X5, X6)	if (m_debugtrace) m_debugtrace->event( NAME, FMT, X1, X2, X3, X4, X5, X6);
#define DEBUG_EVENT2_STR( NAME, FMT, ID, VAL)			if (m_debugtrace) {std::string valstr(VAL); m_debugtrace->event( NAME, FMT, ID, valstr.c_str());}


SegmentProcessor::SegmentProcessor(
		const FeatureConfigMap& featureConfigMap_,
		ErrorBufferInterface* errorhnd_)
	:m_featureConfigMap(&featureConfigMap_)
	,m_concatenatedMap()
	,m_errorhnd(errorhnd_)
	,m_debugtrace(0)
{
	DebugTraceInterface* dbgi = m_errorhnd->debugTrace();
	if (dbgi) m_debugtrace = dbgi->createTraceContext( STRUS_DBGTRACE_COMPONENT_NAME);
}

SegmentProcessor::~SegmentProcessor()
{
	if (m_debugtrace) delete m_debugtrace;
}

void SegmentProcessor::clearTermMaps()
{
	m_concatenatedMap.clear();
	m_searchTerms.clear();
	m_forwardTerms.clear();
	m_metadataTerms.clear();
	m_attributeTerms.clear();
}

void SegmentProcessor::concatDocumentSegment(
		int featidx, std::size_t segmentpos, const char* segmentptr, std::size_t segmentsize)
{
	ConcatenatedMap::iterator ci = m_concatenatedMap.find( featidx);
	if (ci == m_concatenatedMap.end())
	{
		m_concatenatedMap[ featidx]
			= Chunk( std::string( segmentptr, segmentsize), segmentpos);
	}
	else
	{
		Chunk& cm = m_concatenatedMap[ featidx];
		cm.content.push_back(' ');
		std::size_t strpos = cm.content.size();
		cm.content.append( segmentptr, segmentsize);
		cm.concatposmap.push_back( SegPosDef( strpos, strpos+segmentsize, segmentpos));
	}
}

static void fillPositionSet( std::set<analyzer::Position>& pset, std::set<analyzer::Position>& pset_unique, const std::vector<BindTerm>& terms)
{
	std::vector<BindTerm>::const_iterator ri = terms.begin(), re = terms.end();
	for (; ri != re; ++ri)
	{
		if (ri->posbind() == analyzer::BindContent)
		{
			pset.insert( analyzer::Position( ri->seg(), ri->ofs()+1));
		}
		else if (ri->posbind() == analyzer::BindUnique)
		{
			pset_unique.insert( analyzer::Position( ri->seg(), ri->ofs()+1));
		}
	}
}

static void mergeUniquePositionSet( std::set<analyzer::Position>& pset, const std::set<analyzer::Position>& pset_unique)
{
	std::set<analyzer::Position>::const_iterator si = pset.begin(), se = pset.end();
	std::set<analyzer::Position>::const_iterator ui = pset_unique.begin(), ue = pset_unique.end();
	while (si != se && ui != ue)
	{
		while (*si < *ui && si != se) ++si;
		if (si != se)
		{
			for (++ui; ui != ue && *ui <= *si; ++ui){}
			// ... take only the last of a unique sequence
			std::set<analyzer::Position>::const_iterator lastinseq = ui;
			--lastinseq;
			pset.insert( *lastinseq);
		}
	}
	if (ui != ue)
	{
		std::set<analyzer::Position>::const_iterator lastinseq = ue;
		--lastinseq;
		pset.insert( *lastinseq);
	}
}

typedef std::map<analyzer::Position, unsigned int> PositionMap;

static PositionMap getPositionMap( const std::set<analyzer::Position>& pset)
{
	PositionMap posmap;
	std::set<analyzer::Position>::const_iterator pi = pset.begin(), pe = pset.end();
	unsigned int pcnt = 0;
	for (; pi != pe; ++pi)
	{
		posmap[ *pi] = ++pcnt;
	}
	return posmap;
}

static std::set<int> getOrdinalPositionSet(
		const std::vector<BindTerm>& terms,
		const PositionMap& posmap, std::size_t posofs)
{
	std::set<int> rt;
	std::vector<BindTerm>::const_iterator ri = terms.begin(), re = terms.end();
	for (; ri != re; ++ri)
	{
		switch (ri->posbind())
		{
			case analyzer::BindContent:
			{
				PositionMap::const_iterator
					mi = posmap.find( analyzer::Position( ri->seg(), ri->ofs()+1));
				if (mi != posmap.end())
				{
					rt.insert( (int)(mi->second + posofs));
				}
				break;
			}
			case analyzer::BindUnique:
			case analyzer::BindSuccessor:
			{
				PositionMap::const_iterator
					mi = posmap.upper_bound( analyzer::Position( ri->seg(), ri->ofs()));
				if (mi != posmap.end())
				{
					rt.insert( (int)(mi->second + posofs));
				}
				break;
			}
			case analyzer::BindPredecessor:
			{
				PositionMap::const_iterator
					mi = posmap.upper_bound( analyzer::Position( ri->seg(), ri->ofs()+1));
				if (mi != posmap.end() && mi->second + posofs > 1)
				{
					rt.insert( (int)(mi->second + posofs - 1));
				}
				break;
			}
		}
	}
	return rt;
}

static void fillTermsDocument(
		std::vector<analyzer::DocumentTerm>& res,
		const std::vector<BindTerm>& terms,
		const PositionMap& posmap, std::size_t posofs)
{
	std::vector<BindTerm>::const_iterator ri = terms.begin(), re = terms.end();
	for (; ri != re; ++ri)
	{
		switch (ri->posbind())
		{
			case analyzer::BindContent:
			{
				PositionMap::const_iterator
					mi = posmap.find( analyzer::Position( ri->seg(), ri->ofs()+1));
				if (mi != posmap.end())
				{
					res.push_back( analyzer::DocumentTerm( ri->type(), ri->value(), mi->second + posofs));
				}
				break;
			}
			case analyzer::BindUnique:
			case analyzer::BindSuccessor:
			{
				PositionMap::const_iterator
					mi = posmap.upper_bound( analyzer::Position( ri->seg(), ri->ofs()));
				if (mi != posmap.end())
				{
					res.push_back( analyzer::DocumentTerm( ri->type(), ri->value(), mi->second + posofs));
				}
				break;
			}
			case analyzer::BindPredecessor:
			{
				PositionMap::const_iterator
					mi = posmap.upper_bound( analyzer::Position( ri->seg(), ri->ofs()+1));
				if (mi != posmap.end() && mi->second + posofs > 1)
				{
					res.push_back( analyzer::DocumentTerm( ri->type(), ri->value(), mi->second + posofs - 1));
				}
				break;
			}
		}
	}
}

static void fillTermsQuery( 
		std::vector<SegmentProcessor::QueryElement>& res,
		const std::vector<BindTerm>& terms,
		const PositionMap& posmap, std::size_t posofs)
{
	std::vector<BindTerm>::const_iterator ri = terms.begin(), re = terms.end();
	for (; ri != re; ++ri)
	{
		if (ri->posbind() != analyzer::BindContent)
		{
			throw std::runtime_error( _TXT("only position bind content expected in query"));
		}
		PositionMap::const_iterator
			mi = posmap.find( analyzer::Position( ri->seg(), ri->ofs()+1));
		res.push_back( SegmentProcessor::QueryElement( ri->seg(), mi->second + posofs, ri->priority(), analyzer::QueryTerm( ri->type(), ri->value(), ri->ordlen())));
	}
}

void SegmentProcessor::eliminateCovered()
{
	BindTerm::eliminateCoveredElements( m_searchTerms);
	BindTerm::eliminateCoveredElements( m_forwardTerms);
	BindTerm::eliminateCoveredElements( m_attributeTerms);
	BindTerm::eliminateCoveredElements( m_metadataTerms);
}

static analyzer::DocumentStructure::Position getOrdinalPosition( const PositionMap& posmap, const analyzer::Position& pos)
{
	PositionMap::const_iterator pi = posmap.lower_bound( pos);
	if (pi == posmap.end())
	{
		return posmap.empty() ? 1 : posmap.rbegin()->second+1;
	}
	else
	{
		return pi->second;
	}
}

static analyzer::DocumentStructure::PositionRange getOrdinalPositionRange( const PositionMap& posmap, const SearchIndexStructure::PositionRange& posrange)
{
	return analyzer::DocumentStructure::PositionRange( 
			getOrdinalPosition( posmap, posrange.first),
			getOrdinalPosition( posmap, posrange.second));
}

analyzer::Document SegmentProcessor::fetchDocument(
		const std::vector<SeachIndexStructureConfig>& structureConfigs,
		const std::vector<SearchIndexStructure>& structures)
{
	analyzer::Document rt;

	std::set<analyzer::Position> pset;
	std::set<analyzer::Position> pset_unique;
	fillPositionSet( pset, pset_unique, m_searchTerms);
	fillPositionSet( pset, pset_unique, m_forwardTerms);
	mergeUniquePositionSet( pset, pset_unique);
	PositionMap posmap = getPositionMap( pset);

	std::size_t posofs = 0;
	std::vector<analyzer::DocumentTerm> seterms;
	fillTermsDocument( seterms, m_searchTerms, posmap, posofs);
	std::vector<analyzer::DocumentTerm> fwterms;
	fillTermsDocument( fwterms, m_forwardTerms, posmap, posofs);

	std::set<int> pset_search = getOrdinalPositionSet( m_searchTerms, posmap, posofs);

	rt.addSearchIndexTerms( seterms);
	rt.addForwardIndexTerms( fwterms);

	std::vector<BindTerm>::const_iterator mi = m_metadataTerms.begin(), me = m_metadataTerms.end();
	for (; mi != me; ++mi)
	{
		NumericVariant value;
		if (!value.initFromString( mi->value().c_str()))
		{
			throw strus::runtime_error(_TXT("cannot convert normalized item to number (metadata element): %s"), mi->value().c_str());
		}
		rt.setMetaData( mi->type(), value);
	}
	std::string accessListStr;
	std::vector<BindTerm>::const_iterator ai = m_attributeTerms.begin(), ae = m_attributeTerms.end();
	for (; ai != ae; ++ai)
	{
		if (ai->type() == analyzer::Document::attribute_access())
		{
			if (!accessListStr.empty()) accessListStr.push_back(',');
			accessListStr.append( ai->value());
			rt.addAccess( ai->value());
		}
		else
		{
			rt.setAttribute( ai->type(), ai->value());
		}
	}
	if (!accessListStr.empty())
	{
		rt.setAttribute( analyzer::Document::attribute_access(), accessListStr);
	}
	std::vector<SearchIndexStructure>::const_iterator si = structures.begin(), se = structures.end();
	for (; si != se; ++si)
	{
		const std::string&
			nn = structureConfigs[ si->configIdx()].structureName();
		analyzer::DocumentStructure::PositionRange
			hh = getOrdinalPositionRange( posmap, si->source());
		analyzer::DocumentStructure::PositionRange
			cc = getOrdinalPositionRange( posmap, si->sink());
		if (hh.defined() && cc.defined())
		{
			std::set<int>::const_iterator qi = pset_search.lower_bound( (int)hh.start());
			if (qi != pset_search.end() && *qi < (int)hh.end())
			{
				qi = pset_search.lower_bound( (int)cc.start());
				if (qi != pset_search.end() && *qi < (int)cc.end())
				{
					rt.addSearchIndexStructure( nn, hh, cc);
				}
			}
		}
	}
	clearTermMaps();
	return rt;
}

std::vector<SegmentProcessor::QueryElement> SegmentProcessor::fetchQuery() const
{
	std::set<analyzer::Position> pset;
	std::set<analyzer::Position> pset_unique;
	fillPositionSet( pset, pset_unique, m_searchTerms);
	fillPositionSet( pset, pset_unique, m_metadataTerms);
	mergeUniquePositionSet( pset, pset_unique);
	PositionMap posmap = getPositionMap( pset);

	std::vector<QueryElement> rt;
	fillTermsQuery( rt, m_searchTerms, posmap, 0/*position offset*/);
	return rt;
}

static const char* featureClassType( FeatureClass featclass)
{
	switch (featclass)
	{
		case FeatMetaData: return "metadata";
		case FeatAttribute: return "attribute";
		case FeatSearchIndexTerm: return "search index";
		case FeatForwardIndexTerm: return "forward index";
	}
	return 0;
}

void SegmentProcessor::processContentTokens( std::vector<BindTerm>& result, const FeatureConfig& feat, const std::vector<analyzer::Token>& tokens, const char* segsrc, std::size_t segmentpos, const std::vector<SegPosDef>& concatposmap) const
{
	DEBUG_OPEN( featureClassType( feat.featureClass()))
	std::vector<SegPosDef>::const_iterator
		ci = concatposmap.begin(), ce = concatposmap.end();
	std::vector<analyzer::Token>::const_iterator
		ti = tokens.begin(), te = tokens.end();
	for (; ti != te; ++ti)
	{
		// Calculate string position of segment start for concatenated segments:
		int str_position = 0;
		if (ci != ce)
		{
			for (; ci != ce && ci->end_strpos < ti->origpos().ofs(); ++ci){}
			if (ci != ce)
			{
				str_position = ci->start_strpos;
				segmentpos = ci->segpos;
			}
		}
		std::string termval( feat.normalize( segsrc + ti->origpos().ofs(), ti->origsize()));
		if (!termval.empty() && termval[0] == '\0')
		{
			// ... handle normalizers with multiple results
			DEBUG_OPEN( "terms")
			char const* vi = termval.c_str();
			char const* ve = vi + termval.size();
			for (++vi; vi < ve; vi = std::strchr( vi, '\0')+1)
			{
				int ofs = ti->origpos().ofs() - str_position/*ofs*/;
				BindTerm term(
					segmentpos, ofs, ofs + ti->origsize(), 1/*len*/,
					feat.priority(), feat.options().positionBind(),
					feat.name()/*type*/, vi/*value*/);
				DEBUG_EVENT4( "term", "[%d %d] %s '%s'", (int)term.seg(), (int)term.ofs(), term.type().c_str(), term.value().c_str());
				result.push_back( term);
			}
			DEBUG_CLOSE()
		}
		else
		{
			int ofs = ti->origpos().ofs() - str_position/*ofs*/;
			BindTerm term(
				segmentpos, ofs, ofs + ti->origsize(), 1/*len*/,
				feat.priority(), feat.options().positionBind(),
				feat.name()/*type*/, termval/*value*/);
			DEBUG_EVENT4( "term", "[%d %d] %s '%s'", (int)term.seg(), (int)term.ofs(), term.type().c_str(), term.value().c_str());
			result.push_back( term);
		}
	}
	DEBUG_CLOSE()
}

void SegmentProcessor::processDocumentSegment( int featidx, std::size_t segmentpos, const char* segsrc, std::size_t segsrcsize, const std::vector<SegPosDef>& concatposmap)
{
	const FeatureConfig& feat = m_featureConfigMap->featureConfig( featidx);
	DEBUG_EVENT2_STR( "segment", "%s [%s]", feat.name().c_str(), strus::getStringContentStart( std::string( segsrc, segsrcsize), 200));

	std::vector<analyzer::Token> tokens = feat.tokenize( segsrc, segsrcsize);
	switch (feat.featureClass())
	{
		case FeatMetaData:
		{
			processContentTokens( m_metadataTerms, feat, tokens, segsrc, segmentpos, concatposmap);
			break;
		}
		case FeatAttribute:
		{
			processContentTokens( m_attributeTerms, feat, tokens, segsrc, segmentpos, concatposmap);
			break;
		}
		case FeatSearchIndexTerm:
		{
			processContentTokens( m_searchTerms, feat, tokens, segsrc, segmentpos, concatposmap);
			break;
		}
		case FeatForwardIndexTerm:
		{
			processContentTokens( m_forwardTerms, feat, tokens, segsrc, segmentpos, concatposmap);
			break;
		}
	}
}

void SegmentProcessor::processDocumentSegment(
		int featidx,
		std::size_t segmentpos,
		const char* segmentptr,
		std::size_t segmentsize)
{
	processDocumentSegment( featidx, segmentpos, segmentptr, segmentsize, std::vector<SegPosDef>());
}

void SegmentProcessor::processConcatenated()
{
	DEBUG_OPEN( "concat")
	ConcatenatedMap::const_iterator
		ci = m_concatenatedMap.begin(),
		ce = m_concatenatedMap.end();

	for (; ci != ce; ++ci)
	{
		processDocumentSegment(
			ci->first, ci->second.concatposmap.begin()->segpos,
			ci->second.content.c_str(), ci->second.content.size(), ci->second.concatposmap);
	}
	DEBUG_CLOSE()
}



