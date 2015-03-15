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
#ifndef _STRUS_NORMALIZER_CHARACTER_CONVERSIONS_HPP_INCLUDED
#define _STRUS_NORMALIZER_CHARACTER_CONVERSIONS_HPP_INCLUDED
#include "strus/normalizerInterface.hpp"
#include <string>
#include <vector>
#include <map>

namespace strus
{

class CharMap
{
public:
	enum ConvType {DiacriticalUnknown, DiacriticalGerman, Lowercase, Uppercase};

	CharMap(){}
	explicit CharMap( ConvType type)
	{
		load( type);
	}

	void load( ConvType type);
	std::string rewrite( const char* src, std::size_t srcsize) const;

private:
	void set( unsigned int chr, const char* value);
	void set( unsigned int chr, unsigned int mapchr);

	void buildMapDiacritical( ConvType diatype);
	void buildMapTolower();
	void buildMapToupper();

private:
	std::map<unsigned int,std::size_t> m_map;
	std::string m_strings;
};

class LowercaseNormalizer
	:public NormalizerInterface
{
public:
	LowercaseNormalizer()
		:m_map( CharMap::Lowercase){}
	
	/// \brief Destructor
	virtual ~LowercaseNormalizer(){}

	virtual std::string normalize(
			NormalizerInterface::Context*,
			const char* src,
			std::size_t srcsize) const
	{
		return m_map.rewrite( src, srcsize);
	}

private:
	CharMap m_map;
};


class UppercaseNormalizer
	:public NormalizerInterface
{
public:
	UppercaseNormalizer()
		:m_map( CharMap::Uppercase){}

	/// \brief Destructor
	virtual ~UppercaseNormalizer(){}

	virtual std::string normalize(
			NormalizerInterface::Context*,
			const char* src,
			std::size_t srcsize) const
	{
		return m_map.rewrite( src, srcsize);
	}
	
private:
	CharMap m_map;
};


class DiacriticalNormalizer
	:public NormalizerInterface
{
public:
	/// \brief Destructor
	virtual ~DiacriticalNormalizer(){}

	class ThisArgument
		:public NormalizerInterface::Argument
	{
	public:
		ThisArgument( const std::string& language);

		virtual ~ThisArgument()
		{}

		CharMap m_map;
	};
	
	class ThisContext
		:public NormalizerInterface::Context
	{
	public:
		explicit ThisContext( const ThisArgument* arg_)
			:m_map(&arg_->m_map){}
	
		virtual ~ThisContext()
		{}
	
		const CharMap* m_map;
	};
	
	virtual Argument* createArgument( const TextProcessorInterface*, const std::vector<std::string>& arg) const;
	virtual Context* createContext( const Argument* arg) const;

	virtual std::string normalize(
			NormalizerInterface::Context* ctx_,
			const char* src,
			std::size_t srcsize) const
	{
		ThisContext* ctx = reinterpret_cast<ThisContext*>( ctx_);
		return ctx->m_map->rewrite( src, srcsize);
	}
};

}//namespace
#endif

