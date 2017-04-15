/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Implementation of the execution context of a query analyzer
/// \file queryAnalyzerContext.hpp
#include "queryAnalyzerContext.hpp"
#include "queryAnalyzer.hpp"
#include "segmentProcessor.hpp"
#include "private/internationalization.hpp"
#include "private/errorUtils.hpp"
#include <vector>
#include <string>
#include <set>
#include <map>

using namespace strus;

QueryAnalyzerContext::QueryAnalyzerContext( const QueryAnalyzer* analyzer_, ErrorBufferInterface* errorhnd_)
	:m_analyzer(analyzer_)
	,m_fields(),m_groups()
	,m_errorhnd(errorhnd_)
{}

void QueryAnalyzerContext::putField( unsigned int fieldNo, const std::string& fieldType, const std::string& content)
{
	try
	{
		m_fields.push_back( Field( fieldNo, fieldType, content));
	}
	CATCH_ERROR_MAP( _TXT("error defining query field: %s"), *m_errorhnd);
}

void QueryAnalyzerContext::groupElements( unsigned int groupId, const std::vector<unsigned int>& fieldNoList, const GroupBy& groupBy, bool groupSingle)
{
	try
	{
		m_groups.push_back( Group( groupId, fieldNoList, groupBy, groupSingle));
	}
	CATCH_ERROR_MAP( _TXT("error grouping query fields: %s"), *m_errorhnd);
}

struct QueryTreeNode
{
	QueryTreeNode( unsigned int groupId_, unsigned int queryElement_, unsigned int position_, unsigned int nofChild_, unsigned int child_, unsigned int next_)
		:groupId(groupId_),queryElement(queryElement_),position(position_),nofChild(nofChild_),child(child_),next(next_){}
	QueryTreeNode( const QueryTreeNode& o)
		:groupId(o.groupId),queryElement(o.queryElement),position(o.position),nofChild(o.nofChild),child(o.child),next(o.next){}

	unsigned int groupId;
	unsigned int queryElement;
	unsigned int position;
	unsigned int nofChild;
	unsigned int child;
	unsigned int next;
};

struct QueryTree
{
	QueryTree()
		:root(),nodear(){}
	QueryTree( const QueryTree& o)
		:root(o.root),nodear(o.nodear){}

	std::vector<unsigned int> root;
	std::vector<QueryTreeNode> nodear;
};

static void buildQueryTreeNode(
		std::vector<QueryTreeNode>& nodear,
		unsigned int groupId, unsigned int position,
		const std::vector<unsigned int>& children)
{
	nodear.push_back( QueryTreeNode( groupId, std::numeric_limits<unsigned int>::max(), position, children.size(), children.empty()?0:children[0], 0));
	std::vector<unsigned int>::const_iterator ci = children.begin(), ce = children.end();
	for (; ci != ce; ++ci)
	{
		if (nodear[*ci].next) throw strus::runtime_error(_TXT("internal: query tree node linked twice"));
		if (ci+1 != ce) nodear[*ci].next = *(ci+1);
	}
}

static void buildQueryTreeLeaf(
		std::vector<QueryTreeNode>& nodear,
		unsigned int elementidx, unsigned int position)
{
	nodear.push_back( QueryTreeNode( 0, elementidx, position, 0, 0, 0));
}

static std::vector<unsigned int> reduceUnifiedNodes( const std::vector<unsigned int>& args)
{
	std::vector<unsigned int> rt;
	std::set<unsigned int> uniq;
	std::vector<unsigned int>::const_iterator ai = args.begin(), ae = args.end();
	for (; ai != ae; ++ai)
	{
		if (uniq.find( *ai) == uniq.end())
		{
			uniq.insert( *ai);
			rt.push_back( *ai);
		}
	}
	return rt;
}

static QueryTree buildQueryTree(
	const std::vector<QueryAnalyzerContext::Group>& groups,
	const std::vector<QueryAnalyzerContext::Field>& fields,
	const std::vector<SegmentProcessor::QueryElement>& elems)
{
	QueryTree rt;
	typedef std::multimap<unsigned int,unsigned int> FieldElementRangeMap;
	typedef FieldElementRangeMap::const_iterator FieldElementRangeItr;
	typedef std::pair<unsigned int,unsigned int> FieldElementRangeElem;
	typedef std::pair<FieldElementRangeItr,FieldElementRangeItr> FieldElementRange;

	FieldElementRangeMap fieldElementRangeMap;	//< map fieldno to its elements (index in elems)
	std::vector<SegmentProcessor::QueryElement>::const_iterator
		ei = elems.begin(), ee = elems.end();
	for (unsigned int eidx=0; ei != ee; ++ei,++eidx)
	{
		fieldElementRangeMap.insert( FieldElementRangeElem( ei->fieldno(), eidx));
	}

	typedef std::map<unsigned int,unsigned int> ElementRootMap;
	typedef std::pair<unsigned int,unsigned int> ElementRootAssignment;

	ElementRootMap elementRootMap;		//< assigns each leaf element to its build up root node
	ElementRootMap nodeRootMap;		//< assigns each tree node to its build up root node

	// Execute all element groupings bottom up creating the query tree nodes:
	std::vector<QueryAnalyzerContext::Group>::const_iterator gi = groups.begin(), ge = groups.end();
	for (; gi != ge; ++gi)
	{
		// Build up the argument list of the grouping operation.
		// For all elements selected take the current build up root nodes without duplicates as arguments:
		std::vector<unsigned int> args;
		std::vector<ElementRootAssignment> elemAssignments;
		std::vector<unsigned int>::const_iterator
			fi = gi->fieldNoList.begin(), fe = gi->fieldNoList.end();
		for (; fi != fe; ++fi)
		{
			FieldElementRange range = fieldElementRangeMap.equal_range( *fi);
			FieldElementRangeItr ri = range.first, re = range.second;
			for (; ri != re; ++ri)
			{
				unsigned int elemidx = ri->second;
				ElementRootMap::const_iterator ni = elementRootMap.find( elemidx);
				if (ni == elementRootMap.end())
				{
					//... element was not grouped yet, create a new element node in the query tree
					unsigned int newNodeIdx = rt.nodear.size();
					args.push_back( newNodeIdx);
					elementRootMap[ elemidx] = newNodeIdx;
					buildQueryTreeLeaf( rt.nodear, elemidx, elems[elemidx].pos());

					elemAssignments.push_back( ElementRootAssignment( elemidx, newNodeIdx));
				}
				else
				{
					if (args.empty() || args.back() != ni->second)
					{
						args.push_back( ni->second);
					}
					elemAssignments.push_back( ElementRootAssignment( elemidx, ni->second));
				}
			}
		}
		switch (gi->groupBy)
		{
			case QueryAnalyzerContextInterface::GroupByPosition:
			{
				// Group selected nodes with the same position into the same group:
				std::vector<unsigned int>::const_iterator ai = args.begin(), ae = args.end();
				std::map<unsigned int,std::vector<unsigned int> > argmap;
				for (; ai != ae; ++ai)
				{
					argmap[ rt.nodear[ *ai].position].push_back( *ai);
				}
				std::map<unsigned int,std::vector<unsigned int> >::const_iterator mi = argmap.begin(), me = argmap.end();
				for (; mi != me; ++mi)
				{
					std::vector<unsigned int> uargs = reduceUnifiedNodes( mi->second);
					if (uargs.size() > 1 || (gi->groupSingle && uargs.size() >= 1))
					{
						std::vector<unsigned int>::const_iterator
							ui = uargs.begin(), ue = uargs.end();
						for (; ui != ue; ++ui)
						{
							nodeRootMap[ *ui] = rt.nodear.size();
						}
						unsigned int position = mi->first;
						buildQueryTreeNode( rt.nodear, gi->groupId, position, uargs);
					}
				}
				break;
			}
			case QueryAnalyzerContextInterface::GroupAll:
			{
				// Group all selected nodes into the same group:
				std::vector<unsigned int> uargs = reduceUnifiedNodes( args);
				if (uargs.size() > 1 || (gi->groupSingle && uargs.size() >= 1))
				{
					std::vector<unsigned int>::const_iterator
						ui = uargs.begin(), ue = uargs.end();
					for (; ui != ue; ++ui)
					{
						nodeRootMap[ *ui] = rt.nodear.size();
					}
					unsigned int position = rt.nodear[ uargs[0]].position;
					buildQueryTreeNode( rt.nodear, gi->groupId, position, uargs);
				}
				break;
			}
			case QueryAnalyzerContextInterface::GroupEvery:
			{
				// Group every selected node into its own group:
				std::vector<unsigned int> uargs = reduceUnifiedNodes( args);
				std::vector<unsigned int>::const_iterator
					ui = uargs.begin(), ue = uargs.end();
				for (; ui != ue; ++ui)
				{
					nodeRootMap[ *ui] = rt.nodear.size();
					unsigned int position = rt.nodear[ *ui].position;
					std::vector<unsigned int> sargs;
					sargs.push_back( *ui);
					buildQueryTreeNode( rt.nodear, gi->groupId, position, sargs);
				}
			}
		}
		// Update map that assigns every element to its tree root node:
		std::vector<ElementRootAssignment>::const_iterator mi = elemAssignments.begin(), me = elemAssignments.end();
		for (; mi != me; ++mi)
		{
			if (nodeRootMap.find( mi->second) != nodeRootMap.end())
			{
				elementRootMap[ mi->first] = nodeRootMap[ mi->second];
			}
		}
	}
	// Evaluate the root elements:
	std::vector<unsigned int> args;
	ei = elems.begin(), ee = elems.end();
	for (unsigned int eidx=0; ei != ee; ++ei,++eidx)
	{
		ElementRootMap::const_iterator ni = elementRootMap.find( eidx);
		if (ni == elementRootMap.end())
		{
			unsigned int newNodeIdx = rt.nodear.size();
			args.push_back( newNodeIdx);
			buildQueryTreeLeaf( rt.nodear, eidx, ei->pos());
		}
		else
		{
			if (args.empty() || args.back() != ni->second)
			{
				args.push_back( ni->second);
			}
		}
	}
	rt.root = reduceUnifiedNodes( args);
	return rt;
}


std::vector<SegmentProcessor::QueryElement> QueryAnalyzerContext::analyzeQueryFields() const
{
	SegmentProcessor segmentProcessor( m_analyzer->featureConfigMap(), m_analyzer->patternFeatureConfigMap());
	PreProcPatternMatchContextMap preProcPatternMatchContextMap( m_analyzer->preProcPatternMatchConfigMap());
	PostProcPatternMatchContextMap postProcPatternMatchContextMap( m_analyzer->postProcPatternMatchConfigMap());

	std::vector<Field>::const_iterator fi = m_fields.begin(), fe = m_fields.end();
	unsigned int fidx=0;
	for (fidx=0; fi != fe; ++fi,++fidx)
	{
		typedef QueryAnalyzer::FieldTypeFeatureMap FieldTypeFeatureMap;
		std::pair<FieldTypeFeatureMap::const_iterator,FieldTypeFeatureMap::const_iterator>
			range = m_analyzer->fieldTypeFeatureMap().equal_range( fi->fieldType);
		FieldTypeFeatureMap::const_iterator ti = range.first, te = range.second;
		for (;ti != te; ++ti)
		{
			segmentProcessor.processDocumentSegment(
				ti->second/*feature type index*/, fidx/*segment pos = field index*/, 
				fi->content.c_str(), fi->content.size());
		}
	}
	if (!m_analyzer->fieldTypePatternMap().empty()) for (fidx=0,fi=m_fields.begin(); fi != fe; ++fi,++fidx)
	{
		typedef QueryAnalyzer::FieldTypePatternMap FieldTypePatternMap;
		std::pair<FieldTypePatternMap::const_iterator,FieldTypePatternMap::const_iterator>
			range = m_analyzer->fieldTypePatternMap().equal_range( fi->fieldType);
		FieldTypePatternMap::const_iterator ti = range.first, te = range.second;
		for (;ti != te; ++ti)
		{
			PreProcPatternMatchContext& ppctx
				= preProcPatternMatchContextMap.context( ti->second);
			ppctx.process( fidx/*segment pos = field index*/, fi->content.c_str(), fi->content.size());
		}
	}
	// fetch pre processing pattern outputs:
	PreProcPatternMatchContextMap::iterator
		vi = preProcPatternMatchContextMap.begin(),
		ve = preProcPatternMatchContextMap.end();
	for (; vi != ve; ++vi)
	{
		segmentProcessor.processPatternMatchResult( vi->fetchResults());
	}

	// process post processing patterns:
	PostProcPatternMatchContextMap::iterator
		pi = postProcPatternMatchContextMap.begin(),
		pe = postProcPatternMatchContextMap.end();
	for (; pi != pe; ++pi)
	{
		pi->process( segmentProcessor.searchTerms());
		pi->process( segmentProcessor.patternLexemTerms());
		segmentProcessor.processPatternMatchResult( pi->fetchResults());
	}
	return segmentProcessor.fetchQuery();
}

static void query_addMetaData( analyzer::Query& qry, const analyzer::Term& term)
{
	NumericVariant value;
	if (!value.initFromString( term.value().c_str()))
	{
		throw strus::runtime_error(_TXT("cannot convert normalized item to number (metadata element): %s"), term.value().c_str());
	}
	qry.pushMetaData( analyzer::MetaData( term.type(), value));
}

static void buildQueryInstructions( analyzer::Query& qry, const std::vector<SegmentProcessor::QueryElement>& elems, const QueryTree& queryTree, unsigned int nodeidx)
{
	const QueryTreeNode& node = queryTree.nodear[ nodeidx];
	if (node.nofChild)
	{
		unsigned int chld = node.child;
		do
		{
			buildQueryInstructions( qry, elems, queryTree, chld);
			chld = queryTree.nodear[ chld].next;
		} while (chld);
		qry.pushOperator( node.groupId, node.nofChild);
	}
	else
	{
		const SegmentProcessor::QueryElement& elem = elems[ node.queryElement];
		switch (elem.termtype())
		{
			case SegmentProcessor::QueryElement::MetaData:
				query_addMetaData( qry, elem);
				break;
			case SegmentProcessor::QueryElement::Term:
				qry.pushTerm( elem);
				break;
			default:
				throw strus::runtime_error(_TXT("internal: illegal leaf element in query tree"));
		}
	}
}

analyzer::Query QueryAnalyzerContext::analyze()
{
	try
	{
		analyzer::Query rt;
		std::vector<SegmentProcessor::QueryElement> elems = analyzeQueryFields();
		if (m_groups.empty())
		{
			// Groups are empty, so we just copy the elements into the instructions
			// and avoid building the query tree for the same result:
			std::vector<SegmentProcessor::QueryElement>::const_iterator
				ei = elems.begin(), ee = elems.end();
			for ( ;ei!=ee; ++ei)
			{
				switch (ei->termtype())
				{
					case SegmentProcessor::QueryElement::MetaData:
						query_addMetaData( rt, *ei);
						break;
					case SegmentProcessor::QueryElement::Term:
						rt.pushTerm( *ei);
						break;
				}
			}
		}
		else
		{
			QueryTree queryTree = buildQueryTree( m_groups, m_fields, elems);
			std::vector<unsigned int>::const_iterator ri = queryTree.root.begin(), re = queryTree.root.end();
			for (; ri != re; ++ri)
			{
				buildQueryInstructions( rt, elems, queryTree, *ri);
			}
		}
		return rt;
	}
	CATCH_ERROR_MAP_RETURN( _TXT("error analyzing query: %s"), *m_errorhnd, analyzer::Query());
}


