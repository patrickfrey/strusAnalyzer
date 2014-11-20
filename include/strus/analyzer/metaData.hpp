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
#ifndef _STRUS_ANALYZER_METADATA_HPP_INCLUDED
#define _STRUS_ANALYZER_METADATA_HPP_INCLUDED

namespace strus {
namespace analyzer {

class MetaData
{
public:
	MetaData()
		:m_name(0){}
	MetaData( const MetaData& o)
		:m_name(o.m_name),m_value(o.m_value){}
	MetaData( char n, float v)
		:m_name(n),m_value(v){}

	char name() const		{return m_name;}
	const float value() const	{return m_value;}

private:
	char m_name;
	float m_value;
};

}}//namespace
#endif

