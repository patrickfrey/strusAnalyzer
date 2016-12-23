/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "normalizerDictMap.hpp"
#include "strus/normalizerFunctionInterface.hpp"
#include "strus/normalizerFunctionInstanceInterface.hpp"
#include "strus/errorBufferInterface.hpp"
#include "private/errorUtils.hpp"
#include "private/internationalization.hpp"
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
		throw strus::runtime_error( _TXT("error opening file '%s' (errno %u)"), filename.c_str(), errno);
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
			throw strus::runtime_error(_TXT("out of memory reading file"));
		}
	}
	if (!feof( fh))
	{
		unsigned int ec = ::ferror( fh);
		::fclose( fh);
		std::ostringstream msg;
		msg << ec;
		throw strus::runtime_error( _TXT("error opening file '%s' (errno %u)"), filename.c_str(), errno);
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
			throw strus::runtime_error(_TXT("too many term mappings inserted into 'CompactNodeTrie' structure of normalizer 'dictmap'"));
		}
		cc = (*eoln)?(eoln+1):eoln;
	}
}


class DictMapNormalizerInstance
	:public NormalizerFunctionInstanceInterface
{
public:
	/// \brief Destructor
	virtual ~DictMapNormalizerInstance(){}

	DictMapNormalizerInstance( const std::string& filename, const TextProcessorInterface* textproc, ErrorBufferInterface* errorhnd)
		:m_errorhnd(errorhnd)
	{
		m_map.loadFile( textproc->getResourcePath( filename));
	}

	virtual std::string normalize(
			const char* src,
			std::size_t srcsize) const
	{
		try
		{
			std::string key( src, srcsize);
			std::string rt;
			if (m_map.get( key, rt))
			{
				return rt;
			}
			else
			{
				return key;
			}
		}
		CATCH_ERROR_MAP_RETURN( _TXT("error in 'dictmap' normalizer: %s"), *m_errorhnd, std::string());
	}

private:
	DictMap m_map;
	ErrorBufferInterface* m_errorhnd;
};


NormalizerFunctionInstanceInterface* DictMapNormalizerFunction::createInstance( const std::vector<std::string>& args, const TextProcessorInterface* textproc) const
{
	if (args.size() == 0)
	{
		m_errorhnd->report( _TXT("name of file with key values expected as argument for 'DictMap' normalizer"));
		return 0;
	}
	if (args.size() > 1)
	{
		m_errorhnd->report( _TXT("too many arguments for 'DictMap' normalizer"));
		return 0;
	}
	try
	{
		return new DictMapNormalizerInstance( args[0], textproc, m_errorhnd);
	}
	CATCH_ERROR_MAP_RETURN( _TXT("error in 'dictmap' normalizer: %s"), *m_errorhnd, 0);
}


