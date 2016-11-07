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
#include <vector>
#include <string>

/// \brief strus toplevel namespace
namespace strus
{
/// \brief Forward declaration
class QueryAnalyzer;
/// \brief Forward declaration
class ErrorBufferInterface;

/// \brief Implementation of the context for analyzing queries for the strus IR engine
class QueryAnalyzerContext
	:public QueryAnalyzerContextInterface
{
public:
	QueryAnalyzerContext( const QueryAnalyzer* analyzer_, ErrorBufferInterface* errorhnd_);

	virtual ~QueryAnalyzerContext(){}

	virtual void putField( unsigned int fieldno, const std::string& fieldtype, const std::string& content);

	virtual void groupElements( const std::string& name, const std::vector<unsigned int>& fieldnoList, const GroupBy& groupBy, bool groupSingle);

	virtual analyzer::Query analyze();

public:
	struct Field
	{
		unsigned int fieldno;
		std::string fieldtype;
		std::string content;

		Field( unsigned int fieldno_, const std::string& fieldtype_, const std::string& content_)
			:fieldno(fieldno_), fieldtype(fieldtype_), content(content_) {}
		Field( const Field& o)
			:fieldno(o.fieldno), fieldtype(o.fieldtype), content(o.content) {}
	};
	struct Group
	{
		std::string name;
		std::vector<unsigned int> fieldnoList;
		GroupBy groupBy;
		bool groupSingle;

		Group( const std::string& name_, const std::vector<unsigned int>& fieldnoList_, const GroupBy& groupBy_, bool groupSingle_)
			:name(name_), fieldnoList(fieldnoList_), groupBy(groupBy_), groupSingle(groupSingle_) {}
		Group( const Group& o)
			:name(o.name), fieldnoList(o.fieldnoList), groupBy(o.groupBy), groupSingle(o.groupSingle) {}
	};

private:
	const QueryAnalyzer* m_analyzer;
	std::vector<Field> m_fields;
	std::vector<Group> m_groups;
	ErrorBufferInterface* m_errorhnd;
};

}//namespace
#endif

