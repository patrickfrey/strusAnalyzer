/*
---------------------------------------------------------------------
    The C++ library strus implements basic operations to build
    a search engine for structured search on unstructured data.

    Copyright (C) 2015 Patrick Frey

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License as published by the Free Software Foundation; either
    version 3 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

--------------------------------------------------------------------

	The latest version of strus can be found at 'http://github.com/patrickfrey/strus'
	For documentation see 'http://patrickfrey.github.com/strus'

--------------------------------------------------------------------
*/
/// \brief Interface for the execution context of a normalizer function
/// \file normalizerFunctionContextInterface.hpp
#ifndef _STRUS_ANALYZER_NORMALIZER_FUNCTION_CONTEXT_INTERFACE_HPP_INCLUDED
#define _STRUS_ANALYZER_NORMALIZER_FUNCTION_CONTEXT_INTERFACE_HPP_INCLUDED
#include <vector>
#include <string>

/// \brief strus toplevel namespace
namespace strus
{

/// \brief Interface to the context (state) for the execution of a normalizer for one unit (document,query)
class NormalizerFunctionContextInterface
{
public:
	/// \brief Destructor
	virtual ~NormalizerFunctionContextInterface(){}

	/// \brief Normalization of a token
	/// \param[in] src start of the token to normalize
	/// \param[in] srcsize size of the token in bytes
	/// \return list of normalized tokens
	virtual std::string normalize( const char* src, std::size_t srcsize)=0;
};

}//namespace
#endif

