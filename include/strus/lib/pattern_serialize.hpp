/*
 * Copyright (c) 2016 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Library with serialization/deserialization of patterns from/to file
/// \file "pattern_serialize.hpp"
#ifndef _STRUS_ANALYZER_PATTERN_SERIALIZE_LIB_HPP_INCLUDED
#define _STRUS_ANALYZER_PATTERN_SERIALIZE_LIB_HPP_INCLUDED
#include <string>
#include <iostream>

/// \brief strus toplevel namespace
namespace strus {

/// \brief Forward declaration
class PatternTermFeederInterface;
/// \brief Forward declaration
class PatternTermFeederInstanceInterface;
/// \brief Forward declaration
class PatternLexerInterface;
/// \brief Forward declaration
class PatternLexerInstanceInterface;
/// \brief Forward declaration
class PatternMatcherInterface;
/// \brief Forward declaration
class PatternMatcherInstanceInterface;
/// \brief Forward declaration
class ErrorBufferInterface;
/// \brief Forward declaration
struct SerializerData;

/// \class PatternSerializer
/// \brief Object with all interfaces needed for serialization
class PatternSerializer
{
public:
	enum {MaxContentBufSize=1<<20};

public:
	/// \brief Constructor
	PatternSerializer( SerializerData* serializerData_, PatternLexerInstanceInterface* lexer_, PatternMatcherInstanceInterface* matcher_, ErrorBufferInterface* errorhnd_)
		:m_errorhnd(errorhnd_),m_serializerData(serializerData_),m_lexer(lexer_),m_feeder(0),m_matcher(matcher_){}
	/// \brief Constructor
	PatternSerializer( SerializerData* serializerData_, PatternTermFeederInstanceInterface* feeder_, PatternMatcherInstanceInterface* matcher_, ErrorBufferInterface* errorhnd_)
		:m_errorhnd(errorhnd_),m_serializerData(serializerData_),m_lexer(0),m_feeder(feeder_),m_matcher(matcher_){}
	/// \brief Destructor
	~PatternSerializer();

	/// \brief Get the lexer interface to instantiate or 0, if not defined
	PatternLexerInstanceInterface* lexer()		{return m_lexer;}
	/// \brief Get the term feeder interface to instantiate or 0, if not defined
	PatternTermFeederInstanceInterface* feeder()	{return m_feeder;}
	/// \brief Get the pattern matcher interface to instantiate
	PatternMatcherInstanceInterface* matcher()	{return m_matcher;}

	bool close();

private:
	PatternSerializer( const PatternSerializer&){}	//... non copyable
	void operator=( const PatternSerializer&){}	//... non copyable

private:
	ErrorBufferInterface* m_errorhnd;
	SerializerData* m_serializerData;
	PatternLexerInstanceInterface* m_lexer;
	PatternTermFeederInstanceInterface* m_feeder;
	PatternMatcherInstanceInterface* m_matcher;
};

///\brief Defines different types of pattern matchers to serialize
enum PatternSerializerType
{
	PatternMatcherWithLexer,
	PatternMatcherWithFeeder
};

///\brief Evaluate, if a file is a pattern serialization file
///\param[in] filename path to file to check
bool isPatternSerializerFile(
		const std::string& filename,
		ErrorBufferInterface* errorhnd);


///\brief Create a serializer of patterns loaded
///\param[in] filename path to file where to write the output to
///\param[in] serializerType type of serialization
///\param[in] errorhnd error buffer interface
PatternSerializer*
	createPatternSerializer(
		const std::string& filename,
		const PatternSerializerType& serializerType,
		ErrorBufferInterface* errorhnd);

///\brief Create a serializer of patterns loaded as text to a stream
///\param[in] output where to print text output to
///\param[in] serializerType type of serialization
///\param[in] errorhnd error buffer interface
PatternSerializer*
	createPatternSerializerText(
		std::ostream& output,
		const PatternSerializerType& serializerType,
		ErrorBufferInterface* errorhnd);

///\brief Instantiate pattern matching interfaces from serialization
///\param[in] filename path to file where to read the input from
///\param[in] lexer pattern lexer instance interface to instantiate from deserialization
///\param[in] matcher pattern matcher instance interface to instantiate from deserialization
///\param[in] errorhnd error buffer interface
bool loadPatternMatcherFromSerialization(
		const std::string& filename,
		PatternLexerInstanceInterface* lexer,
		PatternMatcherInstanceInterface* matcher,
		ErrorBufferInterface* errorhnd);

///\brief Instantiate pattern matching interfaces from serialization
///\param[in] filename path to file where to read the input from
///\param[in] feeder pattern term feeder instance interface to instantiate from deserialization
///\param[in] matcher pattern matcher instance interface to instantiate from deserialization
///\param[in] errorhnd error buffer interface
bool loadPatternMatcherFromSerialization(
		const std::string& filename,
		PatternTermFeederInstanceInterface* feeder,
		PatternMatcherInstanceInterface* matcher,
		ErrorBufferInterface* errorhnd);
}//namespace
#endif

