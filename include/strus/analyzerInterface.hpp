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

public:
	/// \brief Destructor
	virtual ~AnalyzerInterface(){}

	/// \brief Tokenize a document, assign types to tokens and normalize their values
	/// \param[in] content content string to analyze
	virtual std::vector<Term> analyze(
			const std::string& content) const=0;
};

}//namespace
#endif

