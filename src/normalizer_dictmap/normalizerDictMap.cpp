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
#include "strus/normalizerFunctionInterface.hpp"
#include "strus/normalizerFunctionInstanceInterface.hpp"
#include "strus/normalizerFunctionContextInterface.hpp"
#include "strus/analyzerErrorBufferInterface.hpp"
#include <cstring>
#include <cstring>
#include <cstdio>
#include <cerrno>
#include <sys/stat.h>

using namespace strus;

class DictMap
{
public:
	DictMap(){}
	DictMap( const DictMap& o)
		:m_map(o.m_map),m_value_strings(o.m_value_strings){}
	~DictMap(){}

public:
	void loadFile( const std::string& filename);

	bool set( const std::string& key, const std::string& value);
	bool get( const std::string& key, std::string& value) const;

private:
	conotrie::CompactNodeTrie m_map;
	std::string m_value_strings;
};


bool DictMap::set( const std::string& key, const std::string& value)
{
	std::size_t validx = m_value_strings.size();
	if (!m_map.set( key.c_str(), validx)) return false;
	m_value_strings.append( value);
	m_value_strings.push_back( '\0');
	return true;
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
		if (!set( key, val))
		{
			throw std::runtime_error("too many term mappings inserted into 'CompactNodeTrie' structure of normalizer 'dictmap'");
		}
		cc = (*eoln)?(eoln+1):eoln;
	}
}


class DictMapNormalizerFunctionContext
	:public NormalizerFunctionContextInterface
{
public:
	DictMapNormalizerFunctionContext( const DictMap* map_, AnalyzerErrorBufferInterface* errorhnd)
		:m_map( map_),m_errorhnd(errorhnd){}
	
	virtual std::string normalize(
			const char* src,
			std::size_t srcsize)
	{
		try
		{
			std::string key( src, srcsize);
			std::string rt;
			if (m_map->get( key, rt))
			{
				return rt;
			}
			else
			{
				return key;
			}
		}
		catch (const std::runtime_error& err)
		{
			m_errorhnd->report( std::string( err.what()) + " in 'dictmap' normalizer");
			return std::string();
		}
		catch (const std::bad_alloc&)
		{
			m_errorhnd->report( "out of memory in 'dictmap' normalizer");
			return std::string();
		}
		catch (const std::exception& err)
		{
			m_errorhnd->report( std::string(err.what()) + " uncaught exception in 'dictmap' normalizer");
			return std::string();
		}
	}

private:
	const DictMap* m_map;
	AnalyzerErrorBufferInterface* m_errorhnd;
};

class DictMapNormalizerInstance
	:public NormalizerFunctionInstanceInterface
{
public:
	/// \brief Destructor
	virtual ~DictMapNormalizerInstance(){}

	DictMapNormalizerInstance( const std::string& filename, const TextProcessorInterface* textproc, AnalyzerErrorBufferInterface* errorhnd)
		:m_errorhnd(errorhnd)
	{
		m_map.loadFile( textproc->getResourcePath( filename));
	}

	virtual NormalizerFunctionContextInterface* createFunctionContext() const
	{
		try
		{
			return new DictMapNormalizerFunctionContext( &m_map, m_errorhnd);
		}
		catch (const std::runtime_error& err)
		{
			m_errorhnd->report( std::string( err.what()) + " in 'dictmap' normalizer");
			return 0;
		}
		catch (const std::bad_alloc&)
		{
			m_errorhnd->report( "out of memory in 'dictmap' normalizer");
			return 0;
		}
		catch (const std::exception& err)
		{
			m_errorhnd->report( std::string(err.what()) + " uncaught exception in 'dictmap' normalizer");
			return 0;
		}
	}

private:
	DictMap m_map;
	AnalyzerErrorBufferInterface* m_errorhnd;
};


NormalizerFunctionInstanceInterface* DictMapNormalizerFunction::createInstance( const std::vector<std::string>& args, const TextProcessorInterface* textproc, AnalyzerErrorBufferInterface* errorhnd) const
{
	if (args.size() == 0)
	{
		errorhnd->report( "name of file with key values expected as argument for 'DictMap' normalizer");
		return 0;
	}
	if (args.size() > 1)
	{
		errorhnd->report( "too many arguments for 'DictMap' normalizer");
		return 0;
	}
	try
	{
		return new DictMapNormalizerInstance( args[0], textproc, errorhnd);
	}
	catch (const std::runtime_error& err)
	{
		errorhnd->report( std::string( err.what()) + " in 'dictmap' normalizer");
		return 0;
	}
	catch (const std::bad_alloc&)
	{
		errorhnd->report( "out of memory in 'dictmap' normalizer");
		return 0;
	}
	catch (const std::exception& err)
	{
		errorhnd->report( std::string(err.what()) + " uncaught exception in 'dictmap' normalizer");
		return 0;
	}
}


