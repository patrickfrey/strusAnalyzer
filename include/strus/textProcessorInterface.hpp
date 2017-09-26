/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Interface for the container of all types of functions provided for document and query analysis.
/// \file textProcessorInterface.hpp
#ifndef _STRUS_ANALYZER_TEXT_PROCESSOR_INTERFACE_HPP_INCLUDED
#define _STRUS_ANALYZER_TEXT_PROCESSOR_INTERFACE_HPP_INCLUDED
#include "strus/analyzer/documentClass.hpp"
#include "strus/analyzer/segmenterOptions.hpp"
#include <string>
#include <vector>

/// \brief strus toplevel namespace
namespace strus
{
/// \brief Forward declaration
class DocumentClassDetectorInterface;
/// \brief Forward declaration
class SegmenterInterface;
/// \brief Forward declaration
class NormalizerFunctionInterface;
/// \brief Forward declaration
class TokenizerFunctionInterface;
/// \brief Forward declaration
class AggregatorFunctionInterface;
/// \brief Forward declaration
class PatternLexerInterface;
/// \brief Forward declaration
class PatternMatcherInterface;
/// \brief Forward declaration
class PatternMatcherProgramInterface;
/// \brief Forward declaration
class PatternTermFeederInterface;


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

	/// \brief Get a document segmenter object reference
	/// \param[in] segmenterName name of the segmenter used (if empty, find the first one loaded or the default one)
	/// \return a read only document segmenter reference
	virtual const SegmenterInterface* getSegmenterByName( const std::string& segmenterName) const=0;

	/// \brief Get a document segmenter object reference that is able to process the specified MIME type
	/// \param[in] mimetype MIME type of the document type to process
	/// \return a read only document segmenter reference
	virtual const SegmenterInterface* getSegmenterByMimeType( const std::string& mimetype) const=0;

	/// \brief Get the options for a document segmenter for a specific document type
	/// \param[in] scheme document scheme identifier identifying the type of document and its external structure definition
	virtual analyzer::SegmenterOptions getSegmenterOptions( const std::string& scheme) const=0;

	/// \brief Get a const reference to a tokenizer object that implements the splitting of a text segments into tokens
	/// \return the tokenizer reference
	virtual const TokenizerFunctionInterface* getTokenizer( const std::string& name) const=0;

	/// \brief Get a const reference to a normalizer object that implements the transformation of a token into a term string
	/// \return the normalizer reference
	virtual const NormalizerFunctionInterface* getNormalizer( const std::string& name) const=0;

	/// \brief Get a const reference to a statistics collector function object that implements the collection of some counting of document parts
	/// \return the statistics collector function reference
	virtual const AggregatorFunctionInterface* getAggregator( const std::string& name) const=0;

	/// \brief Get a const reference to a pattern lexer 
	/// \return the pattern lexer
	virtual const PatternLexerInterface* getPatternLexer( const std::string& name) const=0;

	/// \brief Get a const reference to a pattern lexer 
	/// \return the pattern lexer
	virtual const PatternMatcherInterface* getPatternMatcher( const std::string& name) const=0;

	/// \brief Get the default pattern term feeder interface for post processing pattern matching on analyzer output
	/// \return the pattern term feeder
	virtual const PatternTermFeederInterface* getPatternTermFeeder() const=0;

	/// \brief Detect the document class from a document start chunk and set the content description attributes 
	/// \param[in,out] dclass content document class
	/// \param[in] contentBegin start chunk of the document with a reasonable size (e.g. max 1K)
	/// \return true, if the document format was recognized, false else
	virtual bool detectDocumentClass( analyzer::DocumentClass& dclass, const char* contentBegin, std::size_t contentBeginSize) const=0;

	/// \brief Define a content detector
	/// \param[in] tokenizer a tokenizer object (pass ownership)
	virtual void defineDocumentClassDetector( DocumentClassDetectorInterface* detector)=0;

	/// \brief Define a document segmenter by name
	/// \param[in] name name of the document segmenter to define
	/// \param[in] segmenter a document segmenter object (pass ownership)
	virtual void defineSegmenter( const std::string& name, SegmenterInterface* segmenter)=0;

	/// \brief Define segmenter optione by document scheme identifier
	/// \param[in] scheme identifier of the document type
	/// \param[in] options attached to this scheme
	virtual void defineSegmenterOptions( const std::string& scheme, const analyzer::SegmenterOptions& options)=0;

	/// \brief Define a tokenizer by name
	/// \param[in] name name of the tokenizer to define
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

	/// \brief Define a pattern matching lexer by name
	/// \param[in] name name of the lexer to define
	/// \param[in] lexer a lexer object (pass ownership)
	virtual void definePatternLexer( const std::string& name, PatternLexerInterface* lexer)=0;

	/// \brief Define a pattern matcher by name
	/// \param[in] name name of the pattern matcher to define
	/// \param[in] matcher a pattern matcher object (pass ownership)
	virtual void definePatternMatcher( const std::string& name, PatternMatcherInterface* matcher)=0;

	/// \brief Function type for fetching descriptions of available functions
	enum FunctionType
	{
		Segmenter,			///< Addresses a document segmenter
		TokenizerFunction,		///< Addresses a tokenizer
		NormalizerFunction,		///< Addresses a normalizer
		AggregatorFunction,		///< Addresses an aggregator
		PatternLexer,			///< Addresses a pattern lexer
		PatternMatcher			///< Addresses a pattern matcher
	};

	/// \brief Get a list of all functions of a specific type available
	/// \param[in] type type of the function
	/// \return the list of function names
	virtual std::vector<std::string> getFunctionList( const FunctionType& type) const=0;
};

}//namespace
#endif

