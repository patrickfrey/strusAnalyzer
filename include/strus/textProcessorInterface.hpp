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
/// \brief Interface for the container of all types of functions provided for document and query analysis.
/// \file textProcessorInterface.hpp
#ifndef _STRUS_ANALYZER_TEXT_PROCESSOR_INTERFACE_HPP_INCLUDED
#define _STRUS_ANALYZER_TEXT_PROCESSOR_INTERFACE_HPP_INCLUDED
#include <string>

/// \brief strus toplevel namespace
namespace strus
{
/// \brief Forward declaration
class NormalizerFunctionInterface;
/// \brief Forward declaration
class TokenizerFunctionInterface;
/// \brief Forward declaration
class StatisticsFunctionInterface;


/// \class TextProcessorInterface
/// \brief Interface for the object providing tokenizers and normalizers used for creating terms from segments of text and functions for collecting overall document statistics
class TextProcessorInterface
{
public:
	/// \brief Desructor
	virtual ~TextProcessorInterface(){}

	/// \brief Declare a path for locating resource files
	/// \param[in] path path to add
	virtual void addResourcePath( const std::string& path)=0;

	/// \brief Get the absolute path of a resource file
	/// \param[in] filename name of the resource file
	virtual std::string getResourcePath( const std::string& filename) const=0;

	/// \brief Get a const reference to a tokenizer object that implements the splitting of a text segments into tokens
	/// \return the tokenizer reference
	virtual const TokenizerFunctionInterface* getTokenizer( const std::string& name) const=0;

	/// \brief Get a const reference to a normalizer object that implements the transformation of a token into a term string
	/// \return the normalizer reference
	virtual const NormalizerFunctionInterface* getNormalizer( const std::string& name) const=0;

	/// \brief Get a const reference to a statistics collector function object that implements the collection of some counting of document parts
	/// \return the statistics collector function reference
	virtual const StatisticsFunctionInterface* getStatistics( const std::string& name) const=0;

	/// \brief Define a tokenizer by name
	/// \param[in] name name of the normalizer to define
	/// \param[in] tokenizer a static const reference to a tokenizer object
	virtual void defineTokenizer( const std::string& name, const TokenizerFunctionInterface* tokenizer)=0;

	/// \brief Define a normalizer by name
	/// \param[in] name name of the normalizer to define
	/// \param[in] normalizer a static const reference to a normalizer object
	virtual void defineNormalizer( const std::string& name, const NormalizerFunctionInterface* normalizer)=0;

	/// \brief Define a statistics collector function by name
	/// \param[in] name name of the statistics collector function to define
	/// \param[in] statfunc a static const reference to a statistics collector function object
	virtual void defineStatistics( const std::string& name, const StatisticsFunctionInterface* statfunc)=0;
};

}//namespace
#endif

