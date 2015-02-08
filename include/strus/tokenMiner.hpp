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
#ifndef _STRUS_ANALYZER_TOKEN_MINER_HPP_INCLUDED
#define _STRUS_ANALYZER_TOKEN_MINER_HPP_INCLUDED
#include <string>

namespace strus
{
/// \brief Forward declaration
class TokenizerInterface;
/// \brief Forward declaration
class NormalizerInterface;

/// \class TokenMiner
/// \brief The unit defining the methods needed for producing the tokens of document analysis
class TokenMiner
{
public:
	/// \brief Constructor
	TokenMiner( const TokenizerInterface* tokenizer_,
			const NormalizerInterface* normalizer_)
		:m_tokenizer(tokenizer_),m_normalizer(normalizer_){}

	/// \brief Get the tokenizer
	const TokenizerInterface* tokenizer() const	{return m_tokenizer;}
	/// \brief Get the normalizer
	const NormalizerInterface* normalizer() const	{return m_normalizer;}

private:
	const TokenizerInterface* m_tokenizer;
	const NormalizerInterface* m_normalizer;
};

}//namespace
#endif

