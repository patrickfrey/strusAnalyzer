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
#include "strus/analyzer/patternMatcherResult.hpp"
#include <utility>

/// \brief strus toplevel namespace
namespace strus {

/// \brief Forward declaration
class ErrorBufferInterface;
/// \brief Forward declaration
class DebugTraceContextInterface;

/// \brief Result format representation (hidden implementation)
typedef struct PatternResultFormat PatternResultFormat;

/// \brief Context for mapping result format strings (allocator,maps,etc.)
class PatternResultFormatContext
{
public:
	/// \brief Constructor
	/// \param[in] errorhnd_ error buffer interface
	explicit PatternResultFormatContext( ErrorBufferInterface* errorhnd_);
	/// \brief Destructor
	~PatternResultFormatContext();

	/// \brief Map a result to a string
	/// \param[in] fmt result format string
	/// \param[in] nofItems number of pattern match result elements
	/// \param[in] items array of pattern match result elements
	/// \return pattern match result value to use in other result items or finally map with 'PatternResultFormatChunk::parseNext( ResultChunk& result, char const*& src)'
	const char* map( const PatternResultFormat* fmt, const analyzer::PatternMatcherResultItem* items, std::size_t nofItems);

private:
	ErrorBufferInterface* m_errorhnd;			///< error buffer interface
	DebugTraceContextInterface* m_debugtrace;		///< debug trace context
	struct Impl;						///< PIMPL internal representatation
	Impl* m_impl;						///< hidden table implementation
};

/// \brief Interface to map variables to a pointer to string
/// \note The map is used to max variable names comparable by pointers without sring compare
class PatternResultFormatVariableMap
{
public:
	virtual ~PatternResultFormatVariableMap(){}

	virtual const char* getVariable( const std::string& name) const=0;
};

/// \brief Parser for result format strings
class PatternResultFormatTable
{
public:
	PatternResultFormatTable( ErrorBufferInterface* errorhnd_, const PatternResultFormatVariableMap* variableMap_);
	~PatternResultFormatTable();

	/// \brief Create a format string representation out of its source
	/// \note Substituted elements are addressed as identifiers in curly brackets '{' '}'
	/// \note Escaping of '{' and '}' is done with backslash '\', e.g. "\{" or "\}"
	/// \return a {Variable,NULL} terminated array of elements
	const PatternResultFormat* createResultFormat( const char* src);

private:
	ErrorBufferInterface* m_errorhnd;			///< error buffer interface
	const PatternResultFormatVariableMap* m_variableMap;	///< map of variables
	struct Impl;						///< PIMPL internal representatation
	Impl* m_impl;						///< hidden table implementation
};


/// \brief Single chunk of a result format for iterating ans build the pattern match result
struct PatternResultFormatChunk
{
	const char* value;		///< pointer to value (not 0-terminated) assigned to the chunk in case of a constant chunk
	std::size_t valuesize;		///< size of value in bytes
	int start_seg;			///< start segment in case of a chunk referencing content
	int start_pos;			///< start position in case of a chunk referencing content
	int end_seg;			///< end segment in case of a chunk referencing content
	int end_pos;			///< end position in case of a chunk referencing content

	///\ brief Parse the next chunk in a mapped pattern match result value
	static bool parseNext( PatternResultFormatChunk& result, char const*& src);
};


class PatternResultFormatMap
{
public:
	PatternResultFormatMap( ErrorBufferInterface* errorhnd_, const char* src_);
	~PatternResultFormatMap();

	std::string map( const analyzer::PatternMatcherResult& res) const;

private:
	std::string mapItem( const analyzer::PatternMatcherResultItem& res) const;

private:
	ErrorBufferInterface* m_errorhnd;			///< error buffer interface
	struct Impl;						///< PIMPL internal representatation
	Impl* m_impl;						///< hidden table implementation
};

}//namespace
#endif

