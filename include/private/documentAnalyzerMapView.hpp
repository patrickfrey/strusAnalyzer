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
#include "strus/structView.hpp"
#include <string>
#include <vector>

/// \brief strus toplevel namespace
namespace strus {
/// \brief analyzer parameter and return value objects namespace
namespace analyzer {

/// \brief Structure describing the internal representation of one element of a document analyzer map for introspection
/// \note The internal representations may not be suitable for reconstructing the objects
class DocumentAnalyzerMapElementView
	:public StructView
{
public:
	DocumentAnalyzerMapElementView( const std::string& mimeType_, const std::string& schema_, const StructView& analyzer_)
	{
		StructView::operator()( "mimetype", mimeType_)( "schema", schema_)( "analyzer", analyzer_);
	}
};

/// \brief Structure describing the internal representation of a document analyzer map for introspection
/// \note The internal representations may not be suitable for reconstructing the objects
class DocumentAnalyzerMapView
	:public StructView
{
public:
	/// \brief Constructor
	/// \param[in] definitions_ definitions
	DocumentAnalyzerMapView( const StructView& definitions_)
	{
		StructView::operator()( "definition", definitions_);
	}
};

}}//namespace
#endif

