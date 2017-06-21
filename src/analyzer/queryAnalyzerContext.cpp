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
	std::set<unsigned int> uniq;
	std::vector<unsigned int>::const_iterator ai = args.begin(), ae = args.end();
	for (; ai != ae; ++ai)
	{
		uniq.insert( *ai);
	}
	return std::vector<unsigned int>( uniq.begin(), uniq.end());
}

class GroupMemberRelationMap
{
public:
	GroupMemberRelationMap(){}

	void insertMember(
		unsigned int elemidx,
		unsigned int nodeidx);

	void updateElementRoot(
		std::map<unsigned int,unsigned int>& elemRootMap,
		const std::vector<unsigned int>& args,
		unsigned int rootNodeIdx);

private:
	typedef std::multimap<unsigned int,unsigned int> NodeMemberMap;
	typedef NodeMemberMap::const_iterator NodeMemberMapItr;
	typedef std::pair<unsigned int,unsigned int> NodeMemberRelation;
	typedef std::pair<NodeMemberMapItr,NodeMemberMapItr> NodeMemberRange;

	NodeMemberMap m_map;
};

void GroupMemberRelationMap::insertMember( unsigned int elemidx, unsigned int nodeidx)
{
	m_map.insert( NodeMemberRelation( nodeidx, elemidx));
}

void GroupMemberRelationMap::updateElementRoot( std::map<unsigned int,unsigned int>& elemRootMap, const std::vector<unsigned int>& args, unsigned int rootNodeIdx)
{
	std::vector<unsigned int>::const_iterator ai = args.begin(), ae = args.end();
	for (; ai != ae; ++ai)
	{
		NodeMemberRange nodeMemberRange = m_map.equal_range( *ai);
		NodeMemberMapItr ni = nodeMemberRange.first, ne = nodeMemberRange.second;
		if (ni == ne) throw strus::runtime_error(_TXT("internal: badly linked tree structure"));
		for (; ni != ne; ++ni)
		{
			elemRootMap[ ni->second] = rootNodeIdx;
		}
	}
}

class ElementNodeMap
{
public:
	void createElementRoot( unsigned int elemidx, unsigned int nodeidx)
	{
		m_elementRootMap[ elemidx] = nodeidx;
		m_rootElementMap.insert( std::pair<unsigned int,unsigned int>( nodeidx, elemidx));
	}

	unsigned int elementRoot( unsigned int elemidx) const
	{
		std::map<unsigned int,unsigned int>::const_iterator ni = m_elementRootMap.find( elemidx);
		if (ni == m_elementRootMap.end())
		{
			throw strus::runtime_error(_TXT("internal: query tree root element not found"));
		}
		else
		{
			return ni->second;
		}
	}

	void setNewRoot( unsigned int nodeidx, unsigned int newroot)
	{
		if (nodeidx == newroot) return;
		typedef std::multimap<unsigned int,unsigned int>::iterator Itr;
		std::pair<Itr,Itr> range = m_rootElementMap.equal_range( nodeidx);
		for (Itr itr=range.first; itr != range.second; ++itr)
		{
			m_elementRootMap[ itr->second] = newroot;
			m_rootElementMap.insert( std::pair<unsigned int, unsigned int>( newroot, itr->second));
		}
		m_rootElementMap.erase( range.first, range.second);
	}

	void setNewRoot( const std::vector<unsigned int>& nodear, unsigned int newroot)
	{
		std::vector<unsigned int>::const_iterator ni = nodear.begin(), ne = nodear.end();
		for (; ni != ne; ++ni)
		{
			setNewRoot( *ni, newroot);
		}
	}

	std::vector<unsigned int> getRootNodes() const
	{
		std::vector<unsigned int> rt;
		std::multimap<unsigned int,unsigned int>::const_iterator
			ri = m_rootElementMap.begin(), re = m_rootElementMap.end();
		if (ri == re) return rt;
		unsigned int prev = ri->first;
		rt.push_back( prev);
		for (++ri; ri != re; ++ri)
		{
			if (ri->first != prev)
			{
				rt.push_back( prev = ri->first);
			}
		}
		return rt;
	}

private:
	std::map<unsigned int,unsigned int> m_elementRootMap;		//< assigns each leaf element to its build up root node
	std::multimap<unsigned int,unsigned int> m_rootElementMap;	//< assigns each root element to its leaves
};

static QueryTree buildQueryTree(
	const std::vector<QueryAnalyzerContext::Group>& groups,
	const std::vector<QueryAnalyzerContext::Field>& fields,
	const std::vector<SegmentProcessor::QueryElement>& elems)
{
	QueryTree rt;
	typedef std::multimap<unsigned int,unsigned int> FieldElementMap;
	typedef FieldElementMap::const_iterator FieldElementMapItr;
	typedef std::pair<unsigned int,unsigned int> FieldElementRelation;
	typedef std::pair<FieldElementMapItr,FieldElementMapItr> FieldElementRange;

	FieldElementMap fieldElementMap;	//< map fieldno to its elements (index in elems)
	ElementNodeMap elementNodeMap;		//< map elements to nodes and nodes to elements

	std::vector<SegmentProcessor::QueryElement>::const_iterator
		ei = elems.begin(), ee = elems.end();
	for (unsigned int eidx=0; ei != ee; ++ei,++eidx)
	{
		fieldElementMap.insert( FieldElementRelation( ei->fieldno(), eidx));
		elementNodeMap.createElementRoot( eidx, rt.nodear.size());
		buildQueryTreeLeaf( rt.nodear, eidx, elems[eidx].pos());
	}

	// Execute all element groupings bottom up creating the query tree nodes:
	std::vector<QueryAnalyzerContext::Group>::const_iterator gi = groups.begin(), ge = groups.end();
	for (; gi != ge; ++gi)
	{
		// Build up the argument list (set of root nodes of the selected arguments) for the grouping operation
		std::vector<unsigned int> args;			// set of arguments (node indices)
		std::vector<unsigned int>::const_iterator
			fi = gi->fieldNoList.begin(), fe = gi->fieldNoList.end();
		for (; fi != fe; ++fi)
		{
			FieldElementRange range = fieldElementMap.equal_range( *fi);
			FieldElementMapItr ri = range.first, re = range.second;
			// For each element of a selected field:
			for (; ri != re; ++ri)
			{
				unsigned int elemidx = ri->second;
				// Get the assigned node index (current root node the leaf is assigned to):
				unsigned int nodeidx = elementNodeMap.elementRoot( elemidx);

				if (args.empty() || args.back() != nodeidx)
				{
					args.push_back( nodeidx);
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
				// For each selected set with elements sharing the same position:
				std::map<unsigned int,std::vector<unsigned int> >::const_iterator mi = argmap.begin(), me = argmap.end();
				for (; mi != me; ++mi)
				{
					std::vector<unsigned int> uargs = reduceUnifiedNodes( mi->second);
					if (uargs.size() > 1 || (gi->groupSingle && !uargs.empty()))
					{
						elementNodeMap.setNewRoot( uargs, rt.nodear.size());
						unsigned int position = mi->first;
						buildQueryTreeNode( rt.nodear, gi->groupId, position, uargs);
					}
				}
				break;
			}
			case QueryAnalyzerContextInterface::GroupUnique:
			{
				// Group all selected nodes into the same group:
				std::vector<unsigned int> uargs = reduceUnifiedNodes( args);
				if (uargs.size() == 1)
				{
					elementNodeMap.setNewRoot( uargs, rt.nodear.size());
					unsigned int position = rt.nodear[ uargs[0]].position;
					buildQueryTreeNode( rt.nodear, gi->groupId, position, uargs);
				}
				else
				{
					throw strus::runtime_error(_TXT("analyze query fields did not create the unique element required"));
				}
				break;
			}
			case QueryAnalyzerContextInterface::GroupAll:
			{
				// Group all selected nodes into the same group:
				std::vector<unsigned int> uargs = reduceUnifiedNodes( args);
				if (uargs.size() > 1 || (gi->groupSingle && !uargs.empty()))
				{
					elementNodeMap.setNewRoot( uargs, rt.nodear.size());
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
					std::vector<unsigned int> sargs;
					sargs.push_back( *ui);
					elementNodeMap.setNewRoot( sargs, rt.nodear.size());
					unsigned int position = rt.nodear[ *ui].position;
					buildQueryTreeNode( rt.nodear, gi->groupId, position, sargs);
				}
			}
		}
	}
	// Evaluate the set of root elements:
	rt.root = elementNodeMap.getRootNodes();
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
				ti->second/*feature type index*/, fi->fieldNo/*segment pos = fieldNo*/, 
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
			ppctx.process( fi->fieldNo/*segment pos = fieldNo*/, fi->content.c_str(), fi->content.size());
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

static void buildQueryInstructions( analyzer::QueryTermExpression& qry, const std::vector<SegmentProcessor::QueryElement>& elems, const QueryTree& queryTree, unsigned int nodeidx)
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
		qry.pushTerm( elems[ node.queryElement]);
	}
}

analyzer::QueryTermExpression QueryAnalyzerContext::analyze()
{
	try
	{
		analyzer::QueryTermExpression rt;
		std::vector<SegmentProcessor::QueryElement> elems = analyzeQueryFields();
		if (m_groups.empty())
		{
			// Groups are empty, so we just copy the elements into the instructions
			// and avoid building the query tree for the same result:
			std::vector<SegmentProcessor::QueryElement>::const_iterator
				ei = elems.begin(), ee = elems.end();
			for ( ;ei!=ee; ++ei)
			{
				rt.pushTerm( *ei);
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
	CATCH_ERROR_MAP_RETURN( _TXT("error analyzing query: %s"), *m_errorhnd, analyzer::QueryTermExpression());
}


