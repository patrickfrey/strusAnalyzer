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
#ifndef _STRUS_ANALYZER_TOKENIZER_INTERFACE_HPP_INCLUDED
#define _STRUS_ANALYZER_TOKENIZER_INTERFACE_HPP_INCLUDED
#include "strus/analyzer/token.hpp"
#include <vector>
#include <string>

namespace strus {

/// \class TokenizerInterface
/// \brief Interface for tokenization
class TokenizerInterface
{
public:
	/// \brief Destructor
	virtual ~TokenizerInterface(){}

	/// \brief Tokenizer argument base class
	class Argument
	{
	public:
		/// \brief Destructor
		virtual ~Argument(){}
	};

	/// \brief Tokenizer context base class
	class Context
	{
	public:
		/// \brief Destructor
		virtual ~Context(){}
	};

	/// \brief Create the arguments needed for tokenization
	/// \param[in] src arguments for the tokenization as list of strings
	/// \return the argument object to be desposed by the caller with delete if not NULL
	virtual Argument* createArgument( const std::vector<std::string>&) const	{return 0;}

	/// \brief Create the context object needed for tokenization
	/// \param[in] arg the tokenization arguments
	/// \return the context object to be desposed by the caller with delete if not NULL
	virtual Context* createContext( const Argument* arg) const			{return 0;}

	/// \brief Flag defined by tokenizer indicating that different segments defined by the tag hierarchy should be concatenated before tokenization
	/// \remark This flag is needed for context sensitive tokenization like for example recognizing punctuation.
	virtual bool concatBeforeTokenize() const	{return false;}

	/// \brief Tokenize a chunk into a list of sub segments
	/// \param[in] ctx context object for tokenization, if needed. created with createContext(const std::string&)const 
	/// \param[in] src pointer to chunk to tokenize
	/// \param[in] srcsize size of chunk to tokenize in bytes
	virtual std::vector<analyzer::Token>
			tokenize( Context* ctx, const char* src, std::size_t srcsize) const=0;
};

}//namespace
#endif


