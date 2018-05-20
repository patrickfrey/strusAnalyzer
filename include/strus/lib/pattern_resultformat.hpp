/*
 * Copyright (c) 2018 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Library providing function for printing a pattern matcher result with a format string
/// \file "pattern_resultformat.hpp"
#ifndef _STRUS_ANALYZER_PATTERN_RESULTFORMAT_LIB_HPP_INCLUDED
#define _STRUS_ANALYZER_PATTERN_RESULTFORMAT_LIB_HPP_INCLUDED
#include "strus/analyzer/patternMatcherResultItem.hpp"
#include <utility>

/// \brief strus toplevel namespace
namespace strus {

/// \brief Forward declaration
class ErrorBufferInterface;
/// \brief Forward declaration
class DebugTraceContextInterface;

/// \brief Result format representation (hidden implementation)
typedef struct ResultFormat ResultFormat;

/// \brief Context for mapping result format strings (allocator,maps,etc.)
class ResultFormatContext
{
public:
	/// \brief Constructor
	/// \param[in] errorhnd_ error buffer interface
	explicit ResultFormatContext( ErrorBufferInterface* errorhnd_);
	/// \brief Destructor
	~ResultFormatContext();

	/// \brief Map a result to a string
	/// \param[in] fmt result format string
	/// \param[in] nofItems number of pattern match result elements
	/// \param[in] items array of pattern match result elements
	/// \return pattern match result value to use in other result items or finally map with 'ResultFormatChunk::parseNext( ResultChunk& result, char const*& src)'
	const char* map( const ResultFormat* fmt, std::size_t nofItems, const analyzer::PatternMatcherResultItem* items);

private:
	ErrorBufferInterface* m_errorhnd;			///< error buffer interface
	DebugTraceContextInterface* m_debugtrace;		///< debug trace context
	struct Impl;						///< PIMPL internal representatation
	Impl* m_impl;						///< hidden table implementation
};

/// \brief Interface to map variables to a pointer to string
/// \note The map is used to max variable names comparable by pointers without sring compare
class ResultFormatVariableMap
{
public:
	virtual ~ResultFormatVariableMap(){}

	virtual const char* getVariable( const std::string& name) const=0;
};

/// \brief Parser for result format strings
class ResultFormatTable
{
public:
	ResultFormatTable( ErrorBufferInterface* errorhnd_, const ResultFormatVariableMap* variableMap_);
	~ResultFormatTable();

	/// \brief Create a format string representation out of its source
	/// \note Substituted elements are addressed as identifiers in curly brackets '{' '}'
	/// \note Escaping of '{' and '}' is done with backslash '\', e.g. "\{" or "\}"
	/// \return a {Variable,NULL} terminated array of elements
	const ResultFormat* createResultFormat( const char* src);

private:
	ErrorBufferInterface* m_errorhnd;			///< error buffer interface
	const ResultFormatVariableMap* m_variableMap;		///< map of variables
	struct Impl;						///< PIMPL internal representatation
	Impl* m_impl;						///< hidden table implementation
};


/// \brief Single chunk of a result format for iterating ans build the pattern match result
struct ResultFormatChunk
{
	const char* value;		///< pointer to value (not 0-terminated) assigned to the chunk in case of a constant chunk
	std::size_t valuesize;		///< size of value in bytes
	int start_seg;			///< start segment in case of a chunk referencing content
	int start_pos;			///< start position in case of a chunk referencing content
	int end_seg;			///< end segment in case of a chunk referencing content
	int end_pos;			///< end position in case of a chunk referencing content

	///\ brief Parse the next chunk in a mapped pattern match result value
	static bool parseNext( ResultFormatChunk& result, char const*& src);
};

}//namespace
#endif

