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
/// \brief Interface for the container of all types of functions provided for document and query analysis.
/// \file textProcessorInterface.hpp
#ifndef _STRUS_ANALYZER_TEXT_PROCESSOR_INTERFACE_HPP_INCLUDED
#define _STRUS_ANALYZER_TEXT_PROCESSOR_INTERFACE_HPP_INCLUDED
#include <string>
#include <vector>

/// \brief strus toplevel namespace
namespace strus
{
/// \brief Forward declaration
class DocumentClassDetectorInterface;
/// \brief Forward declaration
class NormalizerFunctionInterface;
/// \brief Forward declaration
class TokenizerFunctionInterface;
/// \brief Forward declaration
class AggregatorFunctionInterface;
/// \brief Forward declaration
class DocumentClass;


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
	virtual const AggregatorFunctionInterface* getAggregator( const std::string& name) const=0;

	/// \brief Detect the document class from a document start chunk and set the content description attributes 
	/// \param[in,out] dclass content document class
	/// \param[in] contentBegin start chunk of the document with a reasonable size (e.g. max 1K)
	/// \return true, if the document format was recognized, false else
	virtual bool detectDocumentClass( DocumentClass& dclass, const char* contentBegin, std::size_t contentBeginSize) const=0;

	/// \brief Define a content detector
	/// \param[in] tokenizer a tokenizer object (pass ownership)
	virtual void defineDocumentClassDetector( DocumentClassDetectorInterface* detector)=0;

	/// \brief Define a tokenizer by name
	/// \param[in] name name of the normalizer to define
	/// \param[in] tokenizer a tokenizer object (pass ownership)
	virtual void defineTokenizer( const std::string& name, TokenizerFunctionInterface* tokenizer)=0;

	/// \brief Define a normalizer by name
	/// \param[in] name name of the normalizer to define
	/// \param[in] normalizer a normalizer object (pass ownership)
	virtual void defineNormalizer( const std::string& name, NormalizerFunctionInterface* normalizer)=0;

	/// \brief Define an aggregator function by name
	/// \param[in] name name of the aggregator function to define
	/// \param[in] aggregator an aggregator function object (pass ownership)
	virtual void defineAggregator( const std::string& name, AggregatorFunctionInterface* aggregator)=0;

	/// \brief Function type for fetching descriptions of available functions
	enum FunctionType
	{
		TokenizerFunction,		///< Addresses a tokenizer
		NormalizerFunction,		///< Addresses a normalizer
		AggregatorFunction		///< Addresses am aggregator
	};

	/// \brief Get a list of all functions of a specific type available
	/// \param[in] type type of the function
	/// \return the list of function names
	virtual std::vector<std::string> getFunctionList( FunctionType type) const=0;
};

}//namespace
#endif

