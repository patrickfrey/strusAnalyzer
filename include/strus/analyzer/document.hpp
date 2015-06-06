/*
---------------------------------------------------------------------
    The C++ library strus implements basic operations to build
    a search engine for structured search on unstructured data.

    Copyright (C) 2013,2014 Patrick Frey

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

--------------------------------------------------------------------

	The latest version of strus can be found at 'http://github.com/patrickfrey/strus'
	For documentation see 'http://patrickfrey.github.com/strus'

--------------------------------------------------------------------
*/
/// \brief Structure of a document as result of document analysis
/// \file document.hpp
#ifndef _STRUS_ANALYZER_DOCUMENT_HPP_INCLUDED
#define _STRUS_ANALYZER_DOCUMENT_HPP_INCLUDED
#include <string>
#include <vector>
#include "strus/analyzer/term.hpp"
#include "strus/analyzer/attribute.hpp"
#include "strus/analyzer/metaData.hpp"

/// \brief strus toplevel namespace
namespace strus {
/// \brief analyzer parameter and return value objects namespace
namespace analyzer {

/// \brief Structure of a document created as result of a document analysis
class Document
{
public:
	/// \brief Default constructor
	Document(){}
	/// \brief Copy constructor
	Document( const Document& o)
		:m_subdoctypename(o.m_subdoctypename)
		,m_metadata(o.m_metadata)
		,m_attributes(o.m_attributes)
		,m_searchIndexTerms(o.m_searchIndexTerms)
		,m_forwardIndexTerms(o.m_forwardIndexTerms){}

	/// \brief Get the sub document type name
	const std::string& subDocumentTypeName() const		{return m_subdoctypename;}
	/// \brief Get the list of the attributes defined in this document
	const std::vector<Attribute>& attributes() const	{return m_attributes;}
	/// \brief Get the list of the metadata defined in this document
	const std::vector<MetaData>& metadata() const		{return m_metadata;}
	/// \brief Get the list of the search index terms defined in this document
	const std::vector<Term>& searchIndexTerms() const	{return m_searchIndexTerms;}
	/// \brief Get the list of the forward index terms defined in this document
	const std::vector<Term>& forwardIndexTerms() const	{return m_forwardIndexTerms;}

	/// \brief Set the name of the sub document type as declared in the document analyzer (empty for the main document)
	void setSubDocumentTypeName( const std::string& n)
	{
		m_subdoctypename = n;
	}
	/// \brief Define an attribute of the document
	void setAttribute( const std::string& t, const std::string& v)
	{
		m_attributes.push_back( Attribute( t,v));
	}
	/// \brief Define a meta data element of the document
	/// \param[in] t name of the meta data element
	/// \param[in] v value of the meta data element
	void setMetaData( const std::string& t, const std::string& v)
	{
		m_metadata.push_back( MetaData( t,v));
	}
	/// \brief Define a search index term of the document
	/// \param[in] t type name of the search index term
	/// \param[in] v value of the search index term
	/// \param[in] p position of the search index term in the document (token position not byte position)
	void addSearchIndexTerm( const std::string& t, const std::string& v, unsigned int p)
	{
		m_searchIndexTerms.push_back( Term( t, v, p));
	}
	/// \brief Define a forward index term of the document
	/// \param[in] t type name of the forward index term
	/// \param[in] v value of the forward index term
	/// \param[in] p position of the forward index term in the document (token position not byte position)
	void addForwardIndexTerm( const std::string& t, const std::string& v, unsigned int p)
	{
		m_forwardIndexTerms.push_back( Term( t, v, p));
	}
	/// \brief Clear the document content
	void clear()
	{
		m_subdoctypename.clear();
		m_metadata.clear();
		m_attributes.clear();
		m_searchIndexTerms.clear();
		m_forwardIndexTerms.clear();
	}

	/// \brief Swap a document structure with another
	void swap( Document& o)
	{
		m_subdoctypename.swap(o.m_subdoctypename);
		m_metadata.swap(o.m_metadata);
		m_attributes.swap(o.m_attributes);
		m_searchIndexTerms.swap(o.m_searchIndexTerms);
		m_forwardIndexTerms.swap(o.m_forwardIndexTerms);
	}

private:
	std::string m_subdoctypename;
	std::vector<MetaData> m_metadata;
	std::vector<Attribute> m_attributes;
	std::vector<Term> m_searchIndexTerms;
	std::vector<Term> m_forwardIndexTerms;
};

}}//namespace
#endif

