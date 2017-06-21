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
#include "strus/segmenterInstanceInterface.hpp"
#include "strus/segmenterContextInterface.hpp"
#include <set>
#include <iostream>

using namespace strus;

#undef STRUS_LOWLEVEL_DEBUG

void SegmentProcessor::clearTermMaps()
{
	m_concatenatedMap.clear();
	m_searchTerms.clear();
	m_forwardTerms.clear();
	m_metadataTerms.clear();
	m_attributeTerms.clear();
	m_patternLexemTerms.clear();
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
	bool operator <= (const Position& o) const
	{
		return seg == o.seg ? ofs <= o.ofs : seg <= o.seg;
	}
	bool operator == (const Position& o) const
	{
		return seg == o.seg && ofs == o.ofs;
	}
};

static void fillPositionSet( std::set<Position>& pset, std::set<Position>& pset_unique, const std::vector<BindTerm>& terms)
{
	std::vector<BindTerm>::const_iterator ri = terms.begin(), re = terms.end();
	for (; ri != re; ++ri)
	{
		if (ri->posbind() == analyzer::BindContent)
		{
			pset.insert( Position( ri->seg(), ri->ofs()+1));
		}
		else if (ri->posbind() == analyzer::BindUnique)
		{
			pset_unique.insert( Position( ri->seg(), ri->ofs()+1));
		}
	}
}

static void mergeUniquePositionSet( std::set<Position>& pset, const std::set<Position>& pset_unique)
{
	std::set<Position>::const_iterator si = pset.begin(), se = pset.end();
	std::set<Position>::const_iterator ui = pset_unique.begin(), ue = pset_unique.end();
	while (si != se && ui != ue)
	{
		while (*si < *ui && si != se) ++si;
		if (si != se)
		{
			for (++ui; ui != ue && *ui <= *si; ++ui){}
			// ... take only the last of a unique sequence
			std::set<Position>::const_iterator lastinseq = ui;
			--lastinseq;
			pset.insert( *lastinseq);
		}
	}
	if (ui != ue)
	{
		std::set<Position>::const_iterator lastinseq = ue;
		--lastinseq;
		pset.insert( *lastinseq);
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
					mi = posmap.find( Position( ri->seg(), ri->ofs()+1));
				res.push_back( analyzer::DocumentTerm( ri->type(), ri->value(), mi->second + posofs));
				break;
			}
			case analyzer::BindUnique:
			case analyzer::BindSuccessor:
			{
				PositionMap::const_iterator
					mi = posmap.upper_bound( Position( ri->seg(), ri->ofs()));
				if (mi != posmap.end())
				{
					res.push_back( analyzer::DocumentTerm( ri->type(), ri->value(), mi->second + posofs));
				}
				break;
			}
			case analyzer::BindPredecessor:
			{
				PositionMap::const_iterator
					mi = posmap.upper_bound( Position( ri->seg(), ri->ofs()+1));
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
			throw strus::runtime_error(_TXT("only position bind content expected in query"));
		}
		PositionMap::const_iterator
			mi = posmap.find( Position( ri->seg(), ri->ofs()+1));
		res.push_back( SegmentProcessor::QueryElement( ri->seg(), mi->second + posofs, analyzer::QueryTerm( ri->type(), ri->value(), ri->len())));
	}
}

analyzer::Document SegmentProcessor::fetchDocument()
{
	analyzer::Document rt;

	std::set<Position> pset;
	std::set<Position> pset_unique;
	fillPositionSet( pset, pset_unique, m_searchTerms);
	fillPositionSet( pset, pset_unique, m_forwardTerms);
	mergeUniquePositionSet( pset, pset_unique);
	PositionMap posmap = getPositionMap( pset);

	std::size_t posofs = 0;
	std::vector<analyzer::DocumentTerm> seterms;
	fillTermsDocument( seterms, m_searchTerms, posmap, posofs);
	std::vector<analyzer::DocumentTerm> fwterms;
	fillTermsDocument( fwterms, m_forwardTerms, posmap, posofs);

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

std::vector<SegmentProcessor::QueryElement> SegmentProcessor::fetchQuery() const
{
	std::set<Position> pset;
	std::set<Position> pset_unique;
	fillPositionSet( pset, pset_unique, m_searchTerms);
	fillPositionSet( pset, pset_unique, m_metadataTerms);
	mergeUniquePositionSet( pset, pset_unique);
	PositionMap posmap = getPositionMap( pset);

	std::vector<QueryElement> rt;
	fillTermsQuery( rt, m_searchTerms, posmap, 0/*position offset*/);
	return rt;
}

#ifdef STRUS_LOWLEVEL_DEBUG
static const char* featureClassType( FeatureClass featclass)
{
	switch (featclass)
	{
		case FeatMetaData: return "metadata";
		case FeatAttribute: return "attribute";
		case FeatSearchIndexTerm: return "search index";
		case FeatForwardIndexTerm: return "forward index";
		case FeatPatternLexem: return "pattern lexem";
	}
	return 0;
}
#endif

void SegmentProcessor::processContentTokens( std::vector<BindTerm>& result, const FeatureConfig& feat, const std::vector<analyzer::Token>& tokens, const char* segsrc, std::size_t segmentpos, const std::vector<SegPosDef>& concatposmap) const
{
#ifdef STRUS_LOWLEVEL_DEBUG
	const char* indextype = featureClassType( feat.featureClass());
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
					segmentpos, ti->origpos() - str_position/*ofs*/, 1/*len*/,
					feat.options().positionBind(),
					feat.name()/*type*/, vi/*value*/);
#ifdef STRUS_LOWLEVEL_DEBUG
				std::cout << "add " << indextype << " term " << "[" << term.seg() << ":" << term.ofs() << "] " << term.type() << " " << term.value() << std::endl;
#endif
				result.push_back( term);
			}
		}
		else
		{
			BindTerm term(
				segmentpos, ti->origpos() - str_position/*ofs*/, 1/*len*/,
				feat.options().positionBind(),
				feat.name()/*type*/, termval/*value*/);
#ifdef STRUS_LOWLEVEL_DEBUG
			std::cout << "add " << indextype << " term " << "[" << term.seg() << ":" << term.ofs() << "] " << term.type() << " " << term.value() << std::endl;
#endif
			result.push_back( term);
		}
	}
}

void SegmentProcessor::processDocumentSegment( int featidx, std::size_t segmentpos, const char* segsrc, std::size_t segsrcsize, const std::vector<SegPosDef>& concatposmap)
{
	const FeatureConfig& feat = m_featureConfigMap->featureConfig( featidx);
#ifdef STRUS_LOWLEVEL_DEBUG
	std::cout << "process document segment '" << feat.name() << "': " << std::string(segsrc,segsrcsize>100?100:segsrcsize) << std::endl;
#endif
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
		case FeatPatternLexem:
		{
			processContentTokens( m_patternLexemTerms, feat, tokens, segsrc, segmentpos, concatposmap);
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

void SegmentProcessor::processPatternMatchResult( const std::vector<BindTerm>& result)
{
	std::vector<BindTerm>::const_iterator ri = result.begin(), re = result.end();
	for (; ri != re; ++ri)
	{
		const PatternFeatureConfig* cfg = m_patternFeatureConfigMap->getConfig( ri->type());
		if (cfg) switch (cfg->featureClass())
		{
			case FeatMetaData:
				m_metadataTerms.push_back( BindTerm( ri->seg(), ri->ofs(), ri->len(), cfg->options().positionBind(), cfg->name(), ri->value()));
				break;
			case FeatAttribute:
				m_attributeTerms.push_back( BindTerm( ri->seg(), ri->ofs(), ri->len(), cfg->options().positionBind(), cfg->name(), ri->value()));
				break;
			case FeatSearchIndexTerm:
				m_searchTerms.push_back( BindTerm( ri->seg(), ri->ofs(), ri->len(), cfg->options().positionBind(), cfg->name(), ri->value()));
				break;
			case FeatForwardIndexTerm:
				m_forwardTerms.push_back( BindTerm( ri->seg(), ri->ofs(), ri->len(), cfg->options().positionBind(), cfg->name(), ri->value()));
				break;
			case FeatPatternLexem:
				throw strus::runtime_error(_TXT("internal: illegal feature class for pattern match result"));
		}
	}
}


