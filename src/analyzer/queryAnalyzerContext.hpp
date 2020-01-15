/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Implementation of the execution context of a query analyzer
/// \file queryAnalyzerContext.hpp
#ifndef _STRUS_ANALYZER_QUERY_ANALYZER_CONTEXT_IMPLEMENTATION_HPP_INCLUDED
#define _STRUS_ANALYZER_QUERY_ANALYZER_CONTEXT_IMPLEMENTATION_HPP_INCLUDED
#include "strus/queryAnalyzerContextInterface.hpp"
#include "segmentProcessor.hpp"
#include <vector>
#include <string>

/// \brief strus toplevel namespace
namespace strus
{
/// \brief Forward declaration
class QueryAnalyzerInstance;
/// \brief Forward declaration
class ErrorBufferInterface;
/// \brief Forward declaration
class DebugTraceContextInterface;

/// \brief Implementation of the context for analyzing queries for the strus IR engine
class QueryAnalyzerContext
	:public QueryAnalyzerContextInterface
{
public:
	QueryAnalyzerContext( const QueryAnalyzerInstance* analyzer_, ErrorBufferInterface* errorhnd_);

	virtual ~QueryAnalyzerContext();

	virtual void putField( int fieldNo, const std::string& fieldType, const std::string& content);

	virtual void groupElements( int groupId, const std::vector<int>& fieldNoList, const GroupBy& groupBy, bool groupSingle);

	virtual analyzer::QueryTermExpression analyze();

private:
	std::vector<SegmentProcessor::QueryElement> analyzeQueryFields() const;

public:
	struct Field
	{
		int fieldNo;
		std::string fieldType;
		std::string content;

		Field( int fieldNo_, const std::string& fieldType_, const std::string& content_)
			:fieldNo(fieldNo_), fieldType(fieldType_), content(content_) {}
#if __cplusplus >= 201103L
		Field( Field&& ) = default;
		Field( const Field& ) = default;
		Field& operator= ( Field&& ) = default;
		Field& operator= ( const Field& ) = default;
#else
		Field( const Field& o)
			:fieldNo(o.fieldNo), fieldType(o.fieldType), content(o.content) {}
#endif
	};
	struct Group
	{
		int groupId;
		GroupBy groupBy;
		bool groupSingle;
		std::vector<int> fieldNoList;

		Group( int groupId_, const std::vector<int>& fieldNoList_, const GroupBy& groupBy_, bool groupSingle_)
			:groupId(groupId_), groupBy(groupBy_), groupSingle(groupSingle_), fieldNoList(fieldNoList_) {}
		Group( const Group& o)
			:groupId(o.groupId), groupBy(o.groupBy), groupSingle(o.groupSingle), fieldNoList(o.fieldNoList) {}
#if __cplusplus >= 201103L
		Group( Group&& o) = default;
#endif
	};

private:
	const QueryAnalyzerInstance* m_analyzer;
	std::vector<Field> m_fields;
	std::vector<Group> m_groups;
	ErrorBufferInterface* m_errorhnd;
	DebugTraceContextInterface* m_debugtrace;
};

}//namespace
#endif

