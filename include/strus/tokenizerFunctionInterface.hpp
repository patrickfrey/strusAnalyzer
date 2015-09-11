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
/// \brief Interface for a tokenizer function type
/// \file tokenizerFunctionInterface.hpp
#ifndef _STRUS_ANALYZER_TOKENIZER_FUNCTION_INTERFACE_HPP_INCLUDED
#define _STRUS_ANALYZER_TOKENIZER_FUNCTION_INTERFACE_HPP_INCLUDED
#include <vector>
#include <string>

/// \brief strus toplevel namespace
namespace strus {

/// \brief Forward declaration
class TokenizerFunctionInstanceInterface;
/// \brief Forward declaration
class TextProcessorInterface;
/// \brief Forward declaration
class AnalyzerErrorBufferInterface;

/// \class TokenizerFunctionInterface
/// \brief Interface for a tokenizer function
class TokenizerFunctionInterface
{
public:
	/// \brief Destructor
	virtual ~TokenizerFunctionInterface(){}

	/// \brief Create a parameterizable tokenizer function instance
	/// \param[in] args arguments for the tokenizer function
	/// \param[in] tp text processor reference (for loading resources)
	/// \param[in] errorhnd analyzer error buffer interface for reporting exeptions and errors
	virtual TokenizerFunctionInstanceInterface* createInstance( const std::vector<std::string>& args, const TextProcessorInterface* tp, AnalyzerErrorBufferInterface* errorhnd) const=0;
};

}//namespace
#endif


