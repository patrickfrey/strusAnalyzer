/*
 * Copyright (c) 2019 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef _STRUS_ANALYZER_SEACH_INDEX_STRUCTURE_HPP_INCLUDED
#define _STRUS_ANALYZER_SEACH_INDEX_STRUCTURE_HPP_INCLUDED
#include "strus/documentAnalyzerInstanceInterface.hpp"
#include "strus/reference.hpp"
#include "strus/analyzer/position.hpp"
#include <vector>
#include <string>

namespace strus
{
/// \brief Forward declaration
class ErrorBufferInterface;

class SeachIndexFieldConfig
{
public:
	SeachIndexFieldConfig( const std::string& name_, const std::string& scopeexpr_, const std::string& selectexpr_, const std::string& idexpr_, int scopeIdx_)
		:m_name(name_),m_scopeexpr(scopeexpr_),m_selectexpr(selectexpr_),m_idexpr(idexpr_),m_headerStructureList(),m_contentStructureList(),m_scopeIdx(scopeIdx_){}
	SeachIndexFieldConfig( const SeachIndexFieldConfig& o)
		:m_name(o.m_name),m_scopeexpr(o.m_scopeexpr),m_selectexpr(o.m_selectexpr),m_idexpr(o.m_idexpr),m_headerStructureList(o.m_headerStructureList),m_contentStructureList(o.m_contentStructureList),m_scopeIdx(o.m_scopeIdx){}

	const std::string& name() const				{return m_name;}
	const std::string& scopeexpr() const			{return m_scopeexpr;}
	const std::string& selectexpr() const			{return m_selectexpr;}
	const std::string& idexpr() const			{return m_idexpr;}
	const std::vector<int> headerStructureList() const	{return m_headerStructureList;}
	const std::vector<int> contentStructureList() const	{return m_contentStructureList;}
	int scopeIdx() const					{return m_scopeIdx;}

	void defineHeaderStructureRef( int idx)			{m_headerStructureList.push_back(idx);}
	void defineContentStructureRef( int idx)		{m_contentStructureList.push_back(idx);}

private:
	std::string m_name;
	std::string m_scopeexpr;
	std::string m_selectexpr;
	std::string m_idexpr;
	std::vector<int> m_headerStructureList;
	std::vector<int> m_contentStructureList;
	int m_scopeIdx;
};

class SeachIndexStructureConfig
{
public:
	typedef DocumentAnalyzerInstanceInterface::StructureType Type;

	SeachIndexStructureConfig(
			const std::string& structureName_,
			const std::string& headerName_,
			const std::string& contentName_,
			const Type& structureType_)
		:m_structureName(structureName_),m_headerName(headerName_),m_contentName(contentName_),m_structureType(structureType_){}
	SeachIndexStructureConfig( const SeachIndexStructureConfig& o)
		:m_structureName(o.m_structureName),m_headerName(o.m_headerName),m_contentName(o.m_contentName),m_structureType(o.m_structureType){}

	const std::string& structureName() const	{return m_structureName;}
	const std::string& headerName() const		{return m_headerName;}
	const std::string& contentName() const		{return m_contentName;}
	const Type& structureType() const		{return m_structureType;}

private:
	std::string m_structureName;
	std::string m_headerName;
	std::string m_contentName;
	Type m_structureType;
};

class SearchIndexField
{
public:
	SearchIndexField( int configIdx_, int scopeIdx_)
		:m_configIdx(configIdx_),m_scopeIdx(scopeIdx_),m_id(),m_start(),m_end(){}
#if __cplusplus >= 201103L
	SearchIndexField( SearchIndexField&& ) = default;
	SearchIndexField( const SearchIndexField& ) = default;
	SearchIndexField& operator= ( SearchIndexField&& ) = default;
	SearchIndexField& operator= ( const SearchIndexField& ) = default;
#else
	SearchIndexField( const SearchIndexField& o)
		:m_configIdx(o.m_configIdx),m_scopeIdx(o.m_scopeIdx),m_id(o.m_id),m_start(o.m_start),m_end(o.m_end){}
#endif
	int configIdx() const					{return m_configIdx;}
	int scopeIdx() const					{return m_scopeIdx;}
	const std::string& id() const				{return m_id;}
	const analyzer::Position& start() const			{return m_start;}
	const analyzer::Position& end() const			{return m_end;}

	void setId( const std::string& id_)			{m_id = id_;}
	void setStart( const analyzer::Position& start_)	{m_start = start_;}
	void setEnd( const analyzer::Position& end_)		{m_end = end_;}

private:
	int m_configIdx;
	int m_scopeIdx;
	std::string m_id;
	analyzer::Position m_start;
	analyzer::Position m_end;
};

class SearchIndexStructure
{
public:
	typedef std::pair<analyzer::Position,analyzer::Position> PositionRange;

	SearchIndexStructure( int configIdx_, const PositionRange& source_, const PositionRange& sink_)
		:m_configIdx(configIdx_),m_source(source_),m_sink(sink_){}
	SearchIndexStructure( const SearchIndexStructure& o)
		:m_configIdx(o.m_configIdx),m_source(o.m_source),m_sink(o.m_sink){}

	int configIdx() const					{return m_configIdx;}
	const PositionRange& source() const			{return m_source;}
	const PositionRange& sink() const			{return m_sink;}

private:
	int m_configIdx;
	PositionRange m_source;
	PositionRange m_sink;
};

}//namespace
#endif


