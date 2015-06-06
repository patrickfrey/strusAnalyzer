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
/// \brief Interface for the execution context of a tokenizer function
/// \file tokenizerFunctionContextInterface.hpp
#ifndef _STRUS_ANALYZER_TOKENIZER_FUNCTION_CONTEXT_INTERFACE_HPP_INCLUDED
#define _STRUS_ANALYZER_TOKENIZER_FUNCTION_CONTEXT_INTERFACE_HPP_INCLUDED
#include "strus/analyzer/token.hpp"
#include <utility>
#include <cstddef>
#include <vector>

/// \brief strus toplevel namespace
namespace strus {

/// \brief Interface to the context (state) for the execution of a tokenizer for one unit (document,query)
class TokenizerFunctionContextInterface
{
public:
	/// \brief Destructor
	virtual ~TokenizerFunctionContextInterface(){}

	/// \brief Tokenize a segment into a list of tokens
	/// \param[in] src pointer to segment to tokenize
	/// \param[in] srcsize size of the segment to tokenize in bytes
	virtual std::vector<analyzer::Token> tokenize( const char* src, std::size_t srcsize)=0;
};

}//namespace
#endif


