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
#include "normalizerDictMap.hpp"
#include <cstring>
#include <cstring>
#include <cstdio>
#include <cerrno>
#include <sys/stat.h>

using namespace strus;

void DictMap::set( const std::string& key, const std::string& value)
{
	std::size_t validx = m_value_strings.size();
	m_value_strings.append( value);
	m_value_strings.push_back( '\0');
	m_map.set( key.c_str(), validx);
}

bool DictMap::get( const std::string& key, std::string& value) const
{
	conotrie::CompactNodeTrie::NodeData validx;
	if (!m_map.get( key.c_str(), validx)) return false;
	value.append( m_value_strings.c_str() + validx);
	return true;
}

static std::string readFile( const std::string& filename)
{
	std::string rt;
	FILE* fh = ::fopen( filename.c_str(), "rb");
	if (!fh)
	{
		std::ostringstream msg;
		msg << errno;
		throw std::runtime_error( std::string("error opening file '") + filename + "' (errno " + msg.str() + ")");
	}
	unsigned int nn;
	enum {bufsize=(1<<12)};
	char buf[ bufsize];

	while (!!(nn=::fread( buf, 1, bufsize, fh)))
	{
		try
		{
			rt.append( buf, nn);
		}
		catch (const std::bad_alloc& err)
		{
			::fclose( fh);
			throw err;
		}
	}
	if (!feof( fh))
	{
		unsigned int ec = ::ferror( fh);
		::fclose( fh);
		std::ostringstream msg;
		msg << ec;
		throw std::runtime_error( std::string("error opening file '") + filename + "' (errno " + msg.str() + ")");
	}
	else
	{
		::fclose( fh);
	}
	return rt;
}

void DictMap::loadFile( const std::string& filename)
{
	std::string content = readFile( filename);
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

		std::string val( cc, mid - cc);
		std::string key;
		if (mid != eoln)
		{
			key.append( mid+1, eoln-mid-1);
			if (key.size() && key[ key.size()-1] == '\r')
			{
				key.resize( key.size()-1);
			}
		}
		set( key, val);
		cc = (*eoln)?(eoln+1):eoln;
	}
}


std::string DictMapNormalizer::normalize(
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



