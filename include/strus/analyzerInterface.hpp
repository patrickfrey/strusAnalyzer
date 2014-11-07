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
#ifndef _STRUS_ANALYZER_INTERFACE_HPP_INCLUDED
#define _STRUS_ANALYZER_INTERFACE_HPP_INCLUDED
#include <vector>
#include <string>

namespace strus
{

/// \brief Defines a program for analyzing a source text, splitting it into normalized terms that can be fed to the strus IR engine
class AnalyzerInterface
{
public:
	class Term
	{
	public:
		Term()
			:m_pos(0){}
		Term( const Term& o)
			:m_type(o.m_type),m_value(o.m_value),m_pos(o.m_pos){}
		Term( const std::string& t, const std::string& v, unsigned int p)
			:m_type(t),m_value(v),m_pos(p){}

		const std::string& type() const		{return m_type;}
		const std::string& value() const	{return m_value;}
		unsigned int pos() const		{return m_pos;}

		void setPos( unsigned int pos_)		{m_pos = pos_;}

	private:
		std::string m_type;
		std::string m_value;
		unsigned int m_pos;
	};

	class Attribute
	{
	public:
		Attribute()
			:m_type(0){}
		Attribute( const Attribute& o)
			:m_type(o.m_type),m_value(o.m_value){}
		Attribute( char t, const std::string& v)
			:m_type(t),m_value(v){}

		char type() const			{return m_type;}
		const std::string& value() const	{return m_value;}

	private:
		char m_type;
		std::string m_value;
	};

	class MetaData
	{
	public:
		MetaData()
			:m_type(0){}
		MetaData( const MetaData& o)
			:m_type(o.m_type),m_value(o.m_value){}
		MetaData( char t, float v)
			:m_type(t),m_value(v){}

		char type() const		{return m_type;}
		const float value() const	{return m_value;}

	private:
		char m_type;
		float m_value;
	};

	class Document
	{
	public:
		Document()
			:m_metadata(0){}
		Document( const Document& o)
			:m_metadata(o.m_metadata),m_terms(o.m_terms){}

		const std::vector<Attribute>& attributes() const	{return m_attributes;}
		const std::vector<MetaData>& metadata() const		{return m_metadata;}
		const std::vector<Term>& terms() const			{return m_terms;}

		void addAttribute( char t, const std::string& v)
		{
			m_attributes.push_back( Attribute( t,v));
		}
		void addMetaData( char t, float v)
		{
			m_metadata.push_back( MetaData( t,v));
		}
		void addTerm( const std::string& t, const std::string& v, unsigned int p)
		{
			m_terms.push_back( Term( t, v, p));
		}

	private:
		friend class AnalyzerInterface;
		friend class Analyzer;
		std::vector<MetaData> m_metadata;
		std::vector<Attribute> m_attributes;
		std::vector<Term> m_terms;
	};

public:
	/// \brief Destructor
	virtual ~AnalyzerInterface(){}

	/// \brief Tokenize a document, assign types to tokens and metadata and normalize their values
	/// \param[in] content content string to analyze
	virtual Document analyze( const std::string& content) const=0;

	/// \brief Print the internal representation of the program to 'out'
	/// \param[out] out stream to print the program to
	/// \remark this method is mainly used to testing and has no other purpose
	virtual void print( std::ostream& out) const=0;
};

}//namespace
#endif

