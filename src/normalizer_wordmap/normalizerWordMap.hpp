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
#ifndef _STRUS_NORMALIZER_WORDMAP_HPP_INCLUDED
#define _STRUS_NORMALIZER_WORDMAP_HPP_INCLUDED
#include "compactNodeTrie.hpp"
#include "strus/normalizerInterface.hpp"
#include "strus/private/fileio.hpp"
#include <string>
#include <vector>
#include <sstream>
#include <iostream>

namespace strus
{

class WordMap
{
public:
	WordMap(){}
	WordMap( const WordMap& o)
		:m_map(o.m_map),m_value_strings(o.m_value_strings){}
	~WordMap(){}

public:
	void loadFile( const std::string& filename);

	void set( const std::string& key, const std::string& value);
	bool get( const std::string& key, std::string& value) const;

private:
	conotrie::CompactNodeTrie m_map;
	std::string m_value_strings;
};


class WordMapNormalizer
	:public NormalizerInterface
{
public:
	/// \brief Destructor
	virtual ~WordMapNormalizer(){}

	class Argument
		:public NormalizerInterface::Argument
	{
	public:
		Argument( const Argument& o)
			:m_map(o.m_map){}

		Argument( const std::vector<std::string>& arg)
		{
			if (arg.size() == 0) throw std::runtime_error( "name of file with key values expected as argument for 'wordmap' normalizer");
			if (arg.size() > 1) throw std::runtime_error( "too many arguments for 'wordmap' normalizer");
			m_map.loadFile( arg[0]);
		}

		virtual ~Argument(){}

		const WordMap& map() const	{return m_map;}

	public:
		WordMap m_map;
	};

	class Context
		:public NormalizerInterface::Context
	{
	public:
		Context( const Argument& arg)
			:m_map(&arg.map())
		{}

		virtual ~Context(){}
		bool get( const std::string& key, std::string& value) const
		{
			return m_map->get( key, value);
		}

	private:
		const WordMap* m_map;
	};

	virtual NormalizerInterface::Argument* createArgument( const std::vector<std::string>& arg) const
	{
		return new Argument( arg);
	}

	virtual NormalizerInterface::Context* createContext( const NormalizerInterface::Argument* arg_) const
	{
		const Argument* arg = reinterpret_cast<const Argument*>( arg_);
		return new Context( *arg);
	}

	virtual std::string normalize(
			NormalizerInterface::Context* ctx_,
			const char* src,
			std::size_t srcsize) const;
};

}//namespace
#endif

