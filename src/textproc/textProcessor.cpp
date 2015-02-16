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
#include "textProcessor.hpp"
#include "strus/tokenizerInterface.hpp"
#include "strus/normalizerInterface.hpp"
#include "strus/tokenizer/token.hpp"
#include <stdexcept>
#include <boost/algorithm/string.hpp>

using namespace strus;
using namespace strus::tokenizer;


class EmptyNormalizer
	:public NormalizerInterface
{
public:
	EmptyNormalizer(){}

	virtual std::string normalize( Context*, const char* src, std::size_t srcsize) const
	{
		return std::string();
	}
};

class OrigNormalizer
	:public NormalizerInterface
{
public:
	OrigNormalizer(){}

	virtual std::string normalize( Context*, const char* src, std::size_t srcsize) const
	{
		return std::string( src, srcsize);
	}
};

class ContentTokenizer
	:public TokenizerInterface
{
public:
	ContentTokenizer(){}

	virtual std::vector<tokenizer::Token>
			tokenize( Context* ctx, const char* src, std::size_t srcsize) const
	{
		std::vector<tokenizer::Token> rt;
		rt.push_back( tokenizer::Token( 0, srcsize));
		return rt;
	}
};


static ContentTokenizer contentTokenizer;
static OrigNormalizer origNormalizer;
static EmptyNormalizer emptyNormalizer;

TextProcessor::TextProcessor()
{
	defineTokenizer( "content", &contentTokenizer);
	defineNormalizer( "orig", &origNormalizer);
	defineNormalizer( "empty", &emptyNormalizer);
}

const TokenizerInterface* TextProcessor::getTokenizer( const std::string& name) const
{
	std::map<std::string,const TokenizerInterface*>::const_iterator
		ti = m_tokenizer_map.find( boost::algorithm::to_lower_copy( name));
	if (ti == m_tokenizer_map.end())
	{
		throw std::runtime_error(std::string("no tokenizer defined with name '") + name + "'");
	}
	return ti->second;
}

const NormalizerInterface* TextProcessor::getNormalizer( const std::string& name) const
{
	std::map<std::string,const NormalizerInterface*>::const_iterator
		ni = m_normalizer_map.find( boost::algorithm::to_lower_copy( name));
	if (ni == m_normalizer_map.end())
	{
		throw std::runtime_error(std::string("no normalizer defined with name '") + name + "'");
	}
	return ni->second;
}

void TextProcessor::defineTokenizer( const std::string& name, const TokenizerInterface* tokenizer)
{
	m_tokenizer_map[ boost::algorithm::to_lower_copy( name)] = tokenizer;
}

void TextProcessor::defineNormalizer( const std::string& name, const NormalizerInterface* normalizer)
{
	m_normalizer_map[ boost::algorithm::to_lower_copy( name)] = normalizer;
}


