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
	PatternResultFormatTable( const PatternResultFormatVariableMap* variableMap_, ErrorBufferInterface* errorhnd_);
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
	static bool parseNext( PatternResultFormatChunk& result, char const*& src, ErrorBufferInterface* errorhnd);
};

/// \brief Result format for the output of pattern match results with names of members as variables in curly brackets '{' '}'
class PatternResultFormatMap
{
public:
	/// \brief Constructor
	/// \remark Does not throw, check error buffer passed for errors
	/// \param[in] src_ source with format string with three parts separated by '|'.
	///		The first part is the format string for the result,
	///		the second part is the format string for the result items,
	///		the third part is the separator of result items if there are more than one.
	///	The format strings (first and second part) can contain some of the following variables to be substituted
	///	{ordpos}: ordinal (count) position
	///	{ordlen}: ordinal (count) length
	///	{ordend}: ordinal (count) end position, first position after the match
	///	{startseg}: position of the segment the match started
	///	{startpos}: offset of the match in the matching segment
	///	{endseg}: position of the segment the match ends (first byte after the match)
	///	{endpos}: bytes offset of the match in the matching segment
	///	{abspos}: startseg and startofs added together, if this makes sense 
	///	{abslen}: absolute difference (endseg+endofs) - (startseg+startofs), if this makes sense 
	///	{name}: name of the matched variable or pattern
	///	{value}: value of the matched variable or pattern, either defined by format string or the matching chunk from the original source encoded as PatternResultFormatChunk
	/// \param[in] errorhnd_ error buffer interface to report errors (to be checked after this constructor call)
	PatternResultFormatMap( const char* src_, ErrorBufferInterface* errorhnd_);
	/// \brief Destructor
	~PatternResultFormatMap();

	/// \brief Map a result to a string in a format to be decoded with PatternResultFormatChunk::parseNext
	/// \param[in] res result to map
	/// \return result of the mapping
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

