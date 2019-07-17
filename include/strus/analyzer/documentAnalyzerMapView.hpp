/*
 * Copyright (c) 2018 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Structure describing the internal representation of a document analyzer for introspection
/// \note The internal representations may not be suitable for reconstructing the objects
/// \file documentAnalyzerView.hpp
#ifndef _STRUS_ANALYZER_DOCUMENT_ANALYZER_MAP_VIEW_HPP_INCLUDED
#define _STRUS_ANALYZER_DOCUMENT_ANALYZER_MAP_VIEW_HPP_INCLUDED
#include "strus/analyzer/documentAnalyzerView.hpp"
#include <string>
#include <vector>

/// \brief strus toplevel namespace
namespace strus {
/// \brief analyzer parameter and return value objects namespace
namespace analyzer {

/// \brief Structure describing the internal representation of one element of a document analyzer map for introspection
/// \note The internal representations may not be suitable for reconstructing the objects
class DocumentAnalyzerMapElementView
{
public:
	DocumentAnalyzerMapElementView( const DocumentAnalyzerMapElementView& o)
		:m_mimeType(o.m_mimeType),m_schema(o.m_schema),m_analyzer(o.m_analyzer){}
	DocumentAnalyzerMapElementView( const std::string& mimeType_, const std::string& schema_, const DocumentAnalyzerView& analyzer_)
		:m_mimeType(mimeType_),m_schema(schema_),m_analyzer(analyzer_){}

	const std::string& mimeType() const			{return m_mimeType;}
	const std::string& schema() const			{return m_schema;}
	const DocumentAnalyzerView& analyzer() const		{return m_analyzer;}
	
private:
	std::string m_mimeType;
	std::string m_schema;
	DocumentAnalyzerView m_analyzer;
};

/// \brief Structure describing the internal representation of a document analyzer map for introspection
/// \note The internal representations may not be suitable for reconstructing the objects
class DocumentAnalyzerMapView
{
public:
	/// \brief Default constructor
	DocumentAnalyzerMapView(){}
	/// \brief Copy constructor
	DocumentAnalyzerMapView( const DocumentAnalyzerMapView& o)
		:m_definitions(o.m_definitions)
		{}
	/// \brief Constructor
	/// \param[in] definitions_ definitions
	DocumentAnalyzerMapView( const std::vector<DocumentAnalyzerMapElementView>& definitions_)
		:m_definitions(definitions_)
		{}

	const std::vector<DocumentAnalyzerMapElementView>& definitions() const	{return m_definitions;}

private:
	std::vector<DocumentAnalyzerMapElementView> m_definitions;
};

}}//namespace
#endif

