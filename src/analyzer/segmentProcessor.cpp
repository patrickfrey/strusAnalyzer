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
#include "strus/tokenizerFunctionContextInterface.hpp"
#include "strus/normalizerFunctionContextInterface.hpp"
#include "strus/segmenterInstanceInterface.hpp"
#include "strus/segmenterContextInterface.hpp"
#include <set>

using namespace strus;

#undef STRUS_LOWLEVEL_DEBUG

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

struct Position
{
	unsigned int seg;
	unsigned int ofs;

	Position( unsigned int seg_, unsigned int ofs_)
		:seg(seg_),ofs(ofs_){}
	Position( const Position& o)
		:seg(o.seg),ofs(o.ofs){}

	bool operator < (const Position& o) const
	{
		return seg == o.seg ? ofs < o.ofs : seg < o.seg;
	}
	bool operator == (const Position& o) const
	{
		return seg == o.seg && ofs == o.ofs;
	}
};

static void fillPositionSet( std::set<Position>& pset, const std::vector<BindTerm>& terms)
{
	std::vector<BindTerm>::const_iterator ri = terms.begin(), re = terms.end();
	for (; ri != re; ++ri)
	{
		if (ri->posbind() == analyzer::BindContent)
		{
			pset.insert( Position( ri->seg(), ri->ofs()+1));
		}
	}
}

typedef std::map<Position, unsigned int> PositionMap;

static PositionMap getPositionMap( const std::set<Position>& pset)
{
	PositionMap posmap;
	std::set<Position>::const_iterator pi = pset.begin(), pe = pset.end();
	unsigned int pcnt = 0;
	for (; pi != pe; ++pi)
	{
		posmap[ *pi] = ++pcnt;
	}
	return posmap;
}

static std::vector<std::size_t> getQueryFieldEndPositions( const PositionMap& posmap)
{
	std::vector<std::size_t> fields;
	unsigned int curseg = 0;
	PositionMap::const_iterator pi = posmap.begin(), pe = posmap.end();
	for (; pi != pe; ++pi)
	{
		if (curseg != pi->first.seg)
		{
			while (curseg < pi->first.seg)
			{
				fields.push_back( pi->second);
				++curseg;
			}
		}
	}
	return fields;
}

static void fillTerms( std::vector<analyzer::Term>& res, const std::vector<BindTerm>& terms, const PositionMap& posmap, std::size_t posofs)
{
	std::vector<BindTerm>::const_iterator ri = terms.begin(), re = terms.end();
	for (; ri != re; ++ri)
	{
		switch (ri->posbind())
		{
			case analyzer::BindContent:
			{
				PositionMap::const_iterator
					mi = posmap.find( Position( ri->seg(), ri->ofs()+1));
				res.push_back( analyzer::Term( ri->type(), ri->value(), mi->second + posofs));
				break;
			}
			case analyzer::BindSuccessor:
			{
				PositionMap::const_iterator
					mi = posmap.upper_bound( Position( ri->seg(), ri->ofs()));
				if (mi != posmap.end())
				{
					res.push_back( analyzer::Term( ri->type(), ri->value(), mi->second + posofs));
				}
				break;
			}
			case analyzer::BindPredecessor:
			{
				PositionMap::const_iterator
					mi = posmap.upper_bound( Position( ri->seg(), ri->ofs()+1));
				if (mi != posmap.end() && mi->second + posofs > 1)
				{
					res.push_back( analyzer::Term( ri->type(), ri->value(), mi->second + posofs - 1));
				}
				break;
			}
		}
	}
}

analyzer::Document SegmentProcessor::fetchDocument( const analyzer::Document& prevdoc)
{
	analyzer::Document rt = prevdoc;

	std::set<Position> pset;
	fillPositionSet( pset, m_searchTerms);
	fillPositionSet( pset, m_forwardTerms);
	PositionMap posmap = getPositionMap( pset);

	std::size_t posofs = 0;
	if (prevdoc.searchIndexTerms().size() && prevdoc.searchIndexTerms().back().pos() > posofs)
	{
		posofs = prevdoc.searchIndexTerms().back().pos();
	}
	if (prevdoc.forwardIndexTerms().size() && prevdoc.forwardIndexTerms().back().pos() > posofs)
	{
		posofs = prevdoc.forwardIndexTerms().back().pos();
	}
	std::vector<analyzer::Term> seterms;
	fillTerms( seterms, m_searchTerms, posmap, posofs);
	std::vector<analyzer::Term> fwterms;
	fillTerms( fwterms, m_forwardTerms, posmap, posofs);

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
	std::vector<BindTerm>::const_iterator ai = m_attributeTerms.begin(), ae = m_attributeTerms.end();
	for (; ai != ae; ++ai)
	{
		rt.setAttribute( ai->type(), ai->value());
	}
	clearTermMaps();
	return rt;
}

static void query_addMetaData( unsigned int fieldno, analyzer::Query& qry, const analyzer::Term& term)
{
	NumericVariant value;
	if (!value.initFromString( term.value().c_str()))
	{
		throw strus::runtime_error(_TXT("cannot convert normalized item to number (metadata element): %s"), term.value().c_str());
	}
	qry.addMetaData( fieldno, term.pos(), analyzer::MetaData( term.type(), value));
}

analyzer::Query SegmentProcessor::fetchQuery()
{
	analyzer::Query rt;
	std::set<Position> pset;
	fillPositionSet( pset, m_searchTerms);
	fillPositionSet( pset, m_metadataTerms);
	PositionMap posmap = getPositionMap( pset);

	std::vector<analyzer::Term> seterms;
	fillTerms( seterms, m_searchTerms, posmap, 0/*position offset*/);
	std::vector<analyzer::Term> mtterms;
	fillTerms( mtterms, m_metadataTerms, posmap, 0/*position offset*/);

	std::vector<std::size_t> fieldmap = getQueryFieldEndPositions( posmap);

	unsigned int fieldno = 0;
	std::vector<std::size_t>::const_iterator pi = fieldmap.begin(), pe = fieldmap.end();
	std::vector<analyzer::Term>::const_iterator si = seterms.begin(), se = seterms.end();
	std::vector<analyzer::Term>::const_iterator mi = mtterms.begin(), me = mtterms.end();
	while (mi != me && si != se)
	{
		if (mi->pos() < si->pos())
		{
			while (pi < pe && *pi <= mi->pos())
			{
				++fieldno;
				++pi;
			}
			query_addMetaData( fieldno, rt, *mi++);
		}
		else if (mi->pos() > si->pos())
		{
			while (pi < pe && *pi <= si->pos())
			{
				++fieldno;
				++pi;
			}
			rt.addSearchIndexTerm( fieldno, *si++);
		}
		else/* if (mi->pos() == si->pos())*/
		{
			while (pi < pe && *pi <= si->pos())
			{
				++fieldno;
				++pi;
			}
			query_addMetaData( fieldno, rt, *mi++);
			rt.addSearchIndexTerm( fieldno, *si++);
		}
	}
	while (mi != me)
	{
		while (pi < pe && *pi <= mi->pos())
		{
			++fieldno;
			++pi;
		}
		query_addMetaData( fieldno, rt, *mi++);
	}
	while (si != se)
	{
		while (pi < pe && *pi <= si->pos())
		{
			++fieldno;
			++pi;
		}
		rt.addSearchIndexTerm( fieldno, *si++);
	}
	return rt;
}


void SegmentProcessor::processContentTokens( std::vector<BindTerm>& result, FeatureContext& feat, const std::vector<analyzer::Token>& tokens, const char* segsrc, std::size_t segmentpos, const std::vector<SegPosDef>& concatposmap) const
{
#ifdef STRUS_LOWLEVEL_DEBUG
	const char* indextype = feat.m_config->featureClass()==DocumentAnalyzer::FeatSearchIndexTerm?"search index":"forward index";
#endif
	std::vector<SegPosDef>::const_iterator
		ci = concatposmap.begin(), ce = concatposmap.end();
	std::vector<analyzer::Token>::const_iterator
		ti = tokens.begin(), te = tokens.end();
	for (; ti != te; ++ti)
	{
		// Calculate string position of segment start for concatenated segments:
		unsigned int str_position = 0;
		if (ci != ce)
		{
			for (; ci != ce && ci->end_strpos < ti->origpos(); ++ci){}
			if (ci != ce)
			{
				str_position = ci->start_strpos;
				segmentpos = ci->segpos;
			}
		}
		std::string termval( feat.normalize( segsrc + ti->origpos(), ti->origsize()));
		if (termval.size() && termval[0] == '\0')
		{
			// ... handle normalizers with multiple results
			char const* vi = termval.c_str();
			char const* ve = vi + termval.size();
			for (++vi; vi < ve; vi = std::strchr( vi, '\0')+1)
			{
				BindTerm term(
					segmentpos, ti->origpos() - str_position/*ofs*/,
					feat.m_config->name()/*type*/, vi/*value*/,
					feat.m_config->options().positionBind());
#ifdef STRUS_LOWLEVEL_DEBUG
				std::cout << "add " << indextype << " term " << "[" << term.pos() << "] " << term.type() << " " << term.value() << std::endl;
#endif
				result.push_back( term);
			}
		}
		else
		{
			BindTerm term(
				segmentpos, ti->origpos() - str_position/*ofs*/,
				feat.m_config->name()/*type*/, termval/*value*/,
				feat.m_config->options().positionBind());
#ifdef STRUS_LOWLEVEL_DEBUG
			std::cout << "add " << indextype << " term " << "[" << term.pos() << "] " << term.type() << " " << term.value() << std::endl;
#endif
			result.push_back( term);
		}
	}
}

void SegmentProcessor::processDocumentSegment( int featidx, std::size_t segmentpos, const char* segsrc, std::size_t segsrcsize, const std::vector<SegPosDef>& concatposmap)
{
	FeatureContext& feat = m_featureContextMap.featureContext( featidx);
#ifdef STRUS_LOWLEVEL_DEBUG
	std::cout << "process document segment '" << feat.m_config->name() << "': " << std::string(segsrc,segsrcsize>100?100:segsrcsize) << std::endl;
#endif
	std::vector<analyzer::Token>
		tokens = feat.m_tokenizerContext->tokenize( segsrc, segsrcsize);
	switch (feat.m_config->featureClass())
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
	ConcatenatedMap::const_iterator
		ci = m_concatenatedMap.begin(),
		ce = m_concatenatedMap.end();

	for (; ci != ce; ++ci)
	{
		processDocumentSegment(
			ci->first, ci->second.concatposmap.begin()->segpos,
			ci->second.content.c_str(), ci->second.content.size(), ci->second.concatposmap);
	}
}


