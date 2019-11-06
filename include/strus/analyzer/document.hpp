/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Structure of a document as result of document analysis
/// \file document.hpp
#ifndef _STRUS_ANALYZER_DOCUMENT_HPP_INCLUDED
#define _STRUS_ANALYZER_DOCUMENT_HPP_INCLUDED
#include "strus/numericVariant.hpp"
#include "strus/analyzer/documentTerm.hpp"
#include "strus/analyzer/documentAttribute.hpp"
#include "strus/analyzer/documentMetaData.hpp"
#include "strus/analyzer/documentStructure.hpp"
#include <string>
#include <vector>

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
#if __cplusplus >= 201103L
	Document( Document&& ) = default;
	Document( const Document& ) = default;
	Document& operator= ( Document&& ) = default;
	Document& operator= ( const Document& ) = default;
#else
	Document( const Document& o)
		:m_subdoctypename(o.m_subdoctypename)
		,m_metadata(o.m_metadata)
		,m_attributes(o.m_attributes)
		,m_searchIndexTerms(o.m_searchIndexTerms)
		,m_forwardIndexTerms(o.m_forwardIndexTerms)
		,m_searchIndexStructures(o.m_searchIndexStructures){}
#endif

	/// \brief Get the sub document type name
	const std::string& subDocumentTypeName() const				{return m_subdoctypename;}
	/// \brief Get the list of the attributes defined in this document
	const std::vector<DocumentAttribute>& attributes() const		{return m_attributes;}
	/// \brief Get the list of the metadata defined in this document
	const std::vector<DocumentMetaData>& metadata() const			{return m_metadata;}
	/// \brief Get the list of the search index terms defined in this document
	const std::vector<DocumentTerm>& searchIndexTerms() const		{return m_searchIndexTerms;}
	/// \brief Get the list of the forward index terms defined in this document
	const std::vector<DocumentTerm>& forwardIndexTerms() const		{return m_forwardIndexTerms;}
	/// \brief Get the list of the search index structures defined in this document
	const std::vector<DocumentStructure>& searchIndexStructures() const	{return m_searchIndexStructures;}

	/// \brief Set the name of the sub document type as declared in the document analyzer (empty for the main document)
	void setSubDocumentTypeName( const std::string& n)
	{
		m_subdoctypename = n;
	}
	/// \brief Define an attribute of the document
	void setAttribute( const std::string& t, const std::string& v)
	{
		m_attributes.push_back( DocumentAttribute( t,v));
	}

	/// \brief Define a meta data element of the document
	/// \param[in] t name of the meta data element
	/// \param[in] v value of the meta data element
	void setMetaData( const std::string& t, const NumericVariant& v)
	{
		m_metadata.push_back( DocumentMetaData( t,v));
	}

	/// \brief Define a search index term of the document
	/// \param[in] t type name of the search index term
	/// \param[in] v value of the search index term
	/// \param[in] p position of the search index term in the document (token position not byte position)
	void addSearchIndexTerm( const std::string& t, const std::string& v, unsigned int p)
	{
		m_searchIndexTerms.push_back( DocumentTerm( t, v, p));
	}

	/// \brief Define a list of search index terms of the document
	/// \param[in] terms list of terms to add
	void addSearchIndexTerms( const std::vector<DocumentTerm>& terms)
	{
		m_searchIndexTerms.insert( m_searchIndexTerms.end(), terms.begin(), terms.end());
	}

	/// \brief Define a forward index term of the document
	/// \param[in] t type name of the forward index term
	/// \param[in] v value of the forward index term
	/// \param[in] p position of the forward index term in the document (token position not byte position)
	void addForwardIndexTerm( const std::string& t, const std::string& v, unsigned int p)
	{
		m_forwardIndexTerms.push_back( DocumentTerm( t, v, p));
	}

	/// \brief Define a list of forward index terms of the document
	/// \param[in] terms list of terms to add
	void addForwardIndexTerms( const std::vector<DocumentTerm>& terms)
	{
		m_forwardIndexTerms.insert( m_forwardIndexTerms.end(), terms.begin(), terms.end());
	}

	/// \brief Define a search index structure in the document
	/// \param[in] n label of the structure
	/// \param[in] h header element of the structure
	/// \param[in] h content element of the structure
	void addSearchIndexStructure( const std::string& n, const DocumentStructure::PositionRange& h, const DocumentStructure::PositionRange& c)
	{
		m_searchIndexStructures.push_back( DocumentStructure( n, h, c));
	}
	
	/// \brief Clear the document content
	void clear()
	{
		m_subdoctypename.clear();
		m_metadata.clear();
		m_attributes.clear();
		m_searchIndexTerms.clear();
		m_forwardIndexTerms.clear();
		m_searchIndexStructures.clear();
	}

	/// \brief Swap a document structure with another
	void swap( Document& o)
	{
		m_subdoctypename.swap(o.m_subdoctypename);
		m_metadata.swap(o.m_metadata);
		m_attributes.swap(o.m_attributes);
		m_searchIndexTerms.swap(o.m_searchIndexTerms);
		m_forwardIndexTerms.swap(o.m_forwardIndexTerms);
		m_searchIndexStructures.swap(o.m_searchIndexStructures);
	}

private:
	std::string m_subdoctypename;
	std::vector<DocumentMetaData> m_metadata;
	std::vector<DocumentAttribute> m_attributes;
	std::vector<DocumentTerm> m_searchIndexTerms;
	std::vector<DocumentTerm> m_forwardIndexTerms;
	std::vector<DocumentStructure> m_searchIndexStructures;
};

}}//namespace
#endif

