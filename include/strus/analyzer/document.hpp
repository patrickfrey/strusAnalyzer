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
#ifndef _STRUS_ANALYZER_DOCUMENT_HPP_INCLUDED
#define _STRUS_ANALYZER_DOCUMENT_HPP_INCLUDED
#include <string>
#include <vector>
#include "strus/analyzer/term.hpp"
#include "strus/analyzer/attribute.hpp"
#include "strus/analyzer/metaData.hpp"

namespace strus {
namespace analyzer {

class Document
{
public:
	Document()
		:m_metadata(0){}
	Document( const Document& o)
		:m_metadata(o.m_metadata)
		,m_attributes(o.m_attributes)
		,m_searchIndexTerms(o.m_searchIndexTerms)
		,m_forwardIndexTerms(o.m_forwardIndexTerms){}
	Document(
			const std::vector<MetaData>& metadata_,
			const std::vector<Attribute>& attributes_,
			const std::vector<Term>& searchIndexTerms_,
			const std::vector<Term>& forwardIndexTerms_)
		:m_metadata(metadata_)
		,m_attributes(attributes_)
		,m_searchIndexTerms(searchIndexTerms_)
		,m_forwardIndexTerms(forwardIndexTerms_){}

	const std::vector<Attribute>& attributes() const	{return m_attributes;}
	const std::vector<MetaData>& metadata() const		{return m_metadata;}
	const std::vector<Term>& searchIndexTerms() const	{return m_searchIndexTerms;}
	const std::vector<Term>& forwardIndexTerms() const	{return m_forwardIndexTerms;}

	void setAttribute( const std::string& t, const std::string& v)
	{
		m_attributes.push_back( Attribute( t,v));
	}
	void setMetaData( const std::string& t, const std::string& v)
	{
		m_metadata.push_back( MetaData( t,v));
	}
	void addSearchIndexTerm( const std::string& t, const std::string& v, unsigned int p)
	{
		m_searchIndexTerms.push_back( Term( t, v, p));
	}
	void addForwardIndexTerm( const std::string& t, const std::string& v, unsigned int p)
	{
		m_forwardIndexTerms.push_back( Term( t, v, p));
	}

private:
	std::vector<MetaData> m_metadata;
	std::vector<Attribute> m_attributes;
	std::vector<Term> m_searchIndexTerms;
	std::vector<Term> m_forwardIndexTerms;
};

}}//namespace
#endif

