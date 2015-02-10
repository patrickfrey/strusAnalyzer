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
#ifndef _STRUS_ANALYZER_TEXT_PROCESSOR_INTERFACE_HPP_INCLUDED
#define _STRUS_ANALYZER_TEXT_PROCESSOR_INTERFACE_HPP_INCLUDED
#include <string>

namespace strus
{
/// \brief Forward declaration
class NormalizerInterface;
/// \brief Forward declaration
class TokenizerInterface;

/// \class TextProcessorInterface
/// \brief Interface for the object providing tokenizers and normalizers used for creating terms from chunks of text
class TextProcessorInterface
{
public:
	/// \brief Desructor
	virtual ~TextProcessorInterface(){}

	/// \brief Get a const reference to a tokenizer object that implements the splitting of a text chunk into tokens
	/// \return the tokenizer reference
	virtual const TokenizerInterface* getTokenizer( const std::string& name) const=0;

	/// \brief Get a const reference to a normalizer object that implements the transformation of a token into a term string
	/// \return the normalizer reference
	virtual const NormalizerInterface* getNormalizer( const std::string& name) const=0;

	/// \brief Define a tokenizer by name
	/// \param[in] name name of the normalizer to define
	/// \param[in] tokenizer a static const reference to a tokenizer object
	virtual void defineTokenizer( const std::string& name, const TokenizerInterface* tokenizer)=0;

	/// \brief Define a normalizer by name
	/// \param[in] name name of the normalizer to define
	/// \param[in] normalizer a static const reference to a normalizer object
	virtual void defineNormalizer( const std::string& name, const NormalizerInterface* normalizer)=0;
};

}//namespace
#endif

