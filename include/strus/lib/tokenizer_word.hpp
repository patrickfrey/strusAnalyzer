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
/// \brief Exported functions of the strus analyzer word tokenization library
/// \file tokenizer_word.hpp
#ifndef _STRUS_ANALYZER_TOKENIZER_WORD_LIB_HPP_INCLUDED
#define _STRUS_ANALYZER_TOKENIZER_WORD_LIB_HPP_INCLUDED

/// \brief strus toplevel namespace
namespace strus
{

/// \brief Forward declaration
class TokenizerFunctionInterface;
/// \brief Forward declaration
class AnalyzerErrorBufferInterface;

/// \brief Get the tokenizer type that creates the tokenization of words in the input
/// \return the tokenization function
TokenizerFunctionInterface* createTokenizer_word( AnalyzerErrorBufferInterface* errorhnd);
/// \brief Get the tokenizer type that creates the tokenization as splitting of the input by whitespaces 
/// \return the tokenization function
TokenizerFunctionInterface* createTokenizer_whitespace( AnalyzerErrorBufferInterface* errorhnd);

}//namespace
#endif

