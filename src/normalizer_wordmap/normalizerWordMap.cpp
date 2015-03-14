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
#include "normalizerWordMap.hpp"
#include <cstring>

using namespace strus;

void WordMap::set( const std::string& key, const std::string& value)
{
	std::size_t validx = m_value_strings.size();
	m_value_strings.append( value);
	m_value_strings.push_back( '\0');
	m_map.set( key.c_str(), validx);
}

bool WordMap::get( const std::string& key, std::string& value) const
{
	conotrie::CompactNodeTrie::NodeData validx;
	if (!m_map.get( key.c_str(), validx)) return false;
	value.append( m_value_strings.c_str() + validx);
	return true;
}

void WordMap::loadFile( const std::string& filename)
{
	std::string content;
	unsigned int ec = readFile( filename, content);
	if (ec)
	{
		std::stringstream err;
		err << ec;
		throw std::runtime_error( std::string( "error reading wordmap file '") + filename + "' (error code " + err.str() + ")");
	}
	char delim = ' ';
	char const* cc = content.c_str();
	if (content[0] >= 32 && (content[1] == '\r' || content[1] == '\n'))
	{
		delim = content[0];
		++cc;
		while (*cc == '\r' || *cc == '\n') ++cc;
	}
	while (*cc)
	{
		const char* eoln = std::strchr( cc, '\n');
		if (eoln == 0) eoln = std::strchr( cc, '\0');
		char const* ci = cc;
		for (; ci != eoln && *ci == delim; ++ci){}
		if (ci == eoln)
		{
			cc = (*eoln)?(eoln+1):eoln;
			continue;
		}
		const char* mid = std::strchr( cc, delim);
		if (!mid || mid >= eoln) mid = eoln;

		std::string key( cc, mid - cc);
		std::string val;
		if (mid != eoln)
		{
			val.append( mid+1, eoln-mid-1);
			if (val.size() && val[ val.size()-1] == '\r')
			{
				val.resize( val.size()-1);
			}
		}
		set( key, val);
		cc = (*eoln)?(eoln+1):eoln;
	}
}


std::string WordMapNormalizer::normalize(
		NormalizerInterface::Context* ctx_,
		const char* src,
		std::size_t srcsize) const
{
	const Context* ctx = reinterpret_cast<const Context*>( ctx_);
	std::string rt;
	std::string key( src, srcsize);
	if (ctx->get( key, rt))
	{
		return rt;
	}
	else
	{
		return key;
	}
}



