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
#ifndef _STRUS_ANALYZER_TOKENIZER_CONSTRUCTOR_INTERFACE_HPP_INCLUDED
#define _STRUS_ANALYZER_TOKENIZER_CONSTRUCTOR_INTERFACE_HPP_INCLUDED
#include <vector>
#include <string>

namespace strus {

/// \brief Forward declaration
class TokenizerInterface;
/// \brief Forward declaration
class TextProcessorInterface;

/// \class TokenizerConstructorInterface
/// \brief Interface for the tokenizer constructor
class TokenizerConstructorInterface
{
public:
	/// \brief Destructor
	virtual ~TokenizerConstructorInterface(){}

	/// \brief Create a tokenizer
	/// \param[in] args arguments for the tokenization
	/// \param[in] tp text processor reference (for loading resources)
	virtual TokenizerInterface* create( const std::vector<std::string>& args, const TextProcessorInterface* tp) const=0;
};

}//namespace
#endif


