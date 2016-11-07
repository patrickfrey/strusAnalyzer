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

void QueryAnalyzerContext::putField( unsigned int fieldno, const std::string& fieldtype, const std::string& content)
{
	try
	{
		m_fields.push_back( Field( fieldno, fieldtype, content));
	}
	CATCH_ERROR_MAP( _TXT("error defining query field: %s"), *m_errorhnd);
}

void QueryAnalyzerContext::groupElements( const std::string& name, const std::vector<unsigned int>& fieldnoList, const GroupBy& groupBy, bool groupSingle)
{
	try
	{
		m_groups.push_back( Group( name, fieldnoList, groupBy, groupSingle));
	}
	CATCH_ERROR_MAP( _TXT("error grouping query fields: %s"), *m_errorhnd);
}

struct QueryTreeNode
{
	QueryTreeNode( const char* groupname_, unsigned int queryelement_, unsigned int position_, unsigned int nofChild_, unsigned int child_, unsigned int next_)
		:groupname(groupname_),queryelement(queryelement_),position(position_),nofChild(nofChild_),child(child_),next(next_){}
	QueryTreeNode( const QueryTreeNode& o)
		:groupname(o.groupname),queryelement(o.queryelement),position(o.position),nofChild(o.nofChild),child(o.child),next(o.next){}

	const char* groupname;
	unsigned int queryelement;
	unsigned int position;
	unsigned int nofChild;
	unsigned int child;
	unsigned int next;
};

struct QueryTree
{
	QueryTree()
		:root(0),nodear(){}
	QueryTree( const QueryTree& o)
		:root(o.root),nodear(o.nodear){}

	unsigned int root;
	std::vector<QueryTreeNode> nodear;
};

static void buildQueryTreeNode( std::vector<QueryTreeNode>& nodear, const std::string& groupname, unsigned int position, const std::vector<unsigned int>& children)
{
	nodear.push_back( QueryTreeNode( groupname.c_str(), std::numeric_limits<unsigned int>::max(), position, children.size(), children.empty()?0:children[0], 0));
	std::vector<unsigned int>::const_iterator ci = children.begin(), ce = children.end();
	for (; ci != ce; ++ci)
	{
		if (nodear[*ci].next) throw strus::runtime_error(_TXT("internal: query tree node linked twice"));
		if (ci+1 != ce) nodear[*ci].next = *(ci+1);
	}
}

static void buildQueryTreeLeaf( std::vector<QueryTreeNode>& nodear, unsigned int elementidx, unsigned int position)
{
	nodear.push_back( QueryTreeNode( 0, elementidx, position, 0, 0, 0));
}

/// \brief Build up array of element ranges of the fields
typedef std::pair<unsigned int,unsigned int> ElementRange;
static std::vector<ElementRange> getQueryFieldElementRanges( const analyzer::Query& qry)
{
	std::vector<ElementRange> rt;
	rt.push_back( ElementRange( 0,0));
	std::vector<analyzer::Query::Element>::const_iterator ei = qry.elements().begin(), ee = qry.elements().end();
	for (unsigned int eidx=0; ei != ee; ++ei,++eidx)
	{
		while (rt.size() < ei->fieldno())
		{
			rt.push_back( ElementRange( eidx, eidx));
		}
		rt.back().second = eidx;
	}
	return rt;
}

static std::vector<unsigned int> reduceUnifiedNodes( const std::vector<unsigned int>& args)
{
	std::vector<unsigned int> rt;
	std::set<unsigned int> uniq;
	std::vector<unsigned int>::const_iterator ai = args.begin(), ae = args.end();
	for (; ai != ae; ++ai)
	{
		if (uniq.find( *ai) != uniq.end())
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
	const analyzer::Query& qry)
{
	QueryTree rt;
	std::vector<ElementRange> fieldElementRanges = getQueryFieldElementRanges( qry);

	typedef std::map<unsigned int,unsigned int> ElementRootMap;
	typedef std::pair<unsigned int,unsigned int> ElementRootAssignment;

	ElementRootMap elementRootMap;	//< assigns each leaf element to its build up root node
	ElementRootMap nodeRootMap;	//< assigns each tree node to its build up root node

	// Execute all element groupings bottom up creating the query tree nodes:
	std::vector<QueryAnalyzerContext::Group>::const_iterator gi = groups.begin(), ge = groups.end();
	for (; gi != ge; ++gi)
	{
		// Build up the argument list of the grouping operation.
		// For all elements selected take the current build up root nodes without duplicates as arguments:
		std::vector<unsigned int> args;
		std::vector<ElementRootAssignment> elemAssignments;
		std::vector<unsigned int>::const_iterator
			fi = gi->fieldnoList.begin(), fe = gi->fieldnoList.end();
		for (; fi != fe; ++fi)
		{
			ElementRange range = fieldElementRanges[ *fi];
			unsigned int ei = range.first, ee = range.second;
			for (; ei != ee; ++ei)
			{
				ElementRootMap::const_iterator ni = elementRootMap.find( ei);
				if (ni == elementRootMap.end())
				{
					unsigned int newNodeIdx = rt.nodear.size();
					args.push_back( newNodeIdx);
					elementRootMap[ ei] = newNodeIdx;
					buildQueryTreeLeaf( rt.nodear, ei, qry.elements()[ei].position());

					elemAssignments.push_back( ElementRootAssignment( ei, newNodeIdx));
				}
				else
				{
					if (args.empty() || args.back() != ni->second)
					{
						args.push_back( ni->second);
					}
					elemAssignments.push_back( ElementRootAssignment( ni->first, ni->second));
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
					if (uargs.size() > 1 || gi->groupSingle)
					{
						std::vector<unsigned int>::const_iterator
							ui = uargs.begin(), ue = uargs.end();
						for (; ui != ue; ++ui)
						{
							nodeRootMap[ *ui] = rt.nodear.size();
						}
						unsigned int position = mi->first;
						buildQueryTreeNode( rt.nodear, gi->name, position, uargs);
					}
				}
				break;
			}
			case QueryAnalyzerContextInterface::GroupAll:
			{
				// Group all selected nodes into the same group:
				std::vector<unsigned int> uargs = reduceUnifiedNodes( args);
				if (uargs.size() > 1 || gi->groupSingle)
				{
					std::vector<unsigned int>::const_iterator
						ui = uargs.begin(), ue = uargs.end();
					for (; ui != ue; ++ui)
					{
						nodeRootMap[ *ui] = rt.nodear.size();
					}
					unsigned int position = rt.nodear[ uargs[0]].position;
					buildQueryTreeNode( rt.nodear, gi->name, position, uargs);
				}
				break;
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
	// Group all to one root element if not already unique:
	std::vector<unsigned int> args;
	unsigned int ei = 0, ee = qry.elements().size();
	for (; ei != ee; ++ei)
	{
		ElementRootMap::const_iterator ni = elementRootMap.find( ei);
		if (ni == elementRootMap.end())
		{
			unsigned int newNodeIdx = rt.nodear.size();
			args.push_back( newNodeIdx);
			buildQueryTreeLeaf( rt.nodear, ei, qry.elements()[ei].position());
		}
		else
		{
			if (args.empty() || args.back() != ni->second)
			{
				args.push_back( ni->second);
			}
		}
	}
	args = reduceUnifiedNodes( args);
	if (args.size() > 1)
	{
		buildQueryTreeNode( rt.nodear, "", rt.nodear[ args[0]].position, args);
	}
	// Choose last element as query tree root:
	if (!elementRootMap.empty())
	{
		rt.root = elementRootMap.rbegin()->second;
	}
	return rt;
}

static analyzer::Query analyzeQueryFields( const QueryAnalyzer* analyzer, const std::vector<QueryAnalyzerContext::Field>& fields)
{
	SegmentProcessor segmentProcessor( analyzer->featureConfigMap());
	std::vector<QueryAnalyzerContext::Field>::const_iterator fi = fields.begin(), fe = fields.end();
	for (unsigned int fidx=0; fi != fe; ++fi,++fidx)
	{
		typedef QueryAnalyzer::FieldTypeFeatureMap FieldTypeFeatureMap;
		std::pair<FieldTypeFeatureMap::const_iterator,FieldTypeFeatureMap::const_iterator>
			range = analyzer->fieldTypeFeatureMap().equal_range( fi->fieldtype);
		FieldTypeFeatureMap::const_iterator ti = range.first, te = range.second;
		for (;ti != te; ++ti)
		{
			segmentProcessor.processDocumentSegment(
				ti->second/*feature type index*/, fidx/*segment pos = field index*/, 
				fi->content.c_str(), fi->content.size());
		}
	}
	return segmentProcessor.fetchQuery();
}

static void buildQueryInstructions( analyzer::Query& qry, const QueryTree& queryTree, unsigned int nodeidx)
{
	const QueryTreeNode& node = queryTree.nodear[ nodeidx];
	unsigned int chld = node.child;
	for (; chld; chld = queryTree.nodear[ chld].next)
	{
		buildQueryInstructions( qry, queryTree, chld);
	}
	if (node.groupname)
	{
		qry.pushOperator( node.groupname, node.nofChild);
	}
	else if (node.queryelement)
	{
		analyzer::Query::Element elem = qry.elements()[node.queryelement];
		switch (elem.type())
		{
			case analyzer::Query::Element::MetaData:
				qry.pushMetaDataOperand( node.queryelement);
				break;
			case analyzer::Query::Element::SearchIndexTerm:
				qry.pushSearchIndexTermOperand( node.queryelement);
				break;
			default:
				throw strus::runtime_error(_TXT("internal: illegal leaf element in query tree"));
		}
	}
	else
	{
		throw strus::runtime_error(_TXT("internal: corrupt node in query"));
	}
}

analyzer::Query QueryAnalyzerContext::analyze()
{
	try
	{
		analyzer::Query rt = analyzeQueryFields( m_analyzer, m_fields);
		QueryTree queryTree = buildQueryTree( m_groups, m_fields, rt);
		buildQueryInstructions( rt, queryTree, queryTree.root);
		return rt;
	}
	CATCH_ERROR_MAP_RETURN( _TXT("error analyzing query: %s"), *m_errorhnd, analyzer::Query());
}


