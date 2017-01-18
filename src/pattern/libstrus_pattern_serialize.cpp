/*
 * Copyright (c) 2016 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Library with serialization/deserialization of patterns from/to file
/// \file "pattern_serialize.hpp"
#include "strus/lib/pattern_serialize.hpp"
#include "strus/errorBufferInterface.hpp"
#include "strus/patternTermFeederInterface.hpp"
#include "strus/patternTermFeederInstanceInterface.hpp"
#include "strus/patternLexerInterface.hpp"
#include "strus/patternLexerInstanceInterface.hpp"
#include "strus/patternMatcherInterface.hpp"
#include "strus/patternMatcherInstanceInterface.hpp"
#include "strus/base/dll_tags.hpp"
#include "strus/base/symbolTable.hpp"
#include "strus/base/inputStream.hpp"
#include "strus/base/hton.hpp"
#include "strus/base/fileio.hpp"
#include "strus/base/stdint.h"
#include "strus/reference.hpp"
#include "strus/versionAnalyzer.hpp"
#include "private/internationalization.hpp"
#include "private/errorUtils.hpp"
#include "private/utils.hpp"
#include <vector>
#include <string>
#include <memory>
#include <cstring>

using namespace strus;

#define DATA_HEADER_IDSTRING "pattern_serialize\n\0"
static bool g_intl_initialized = false;

namespace strus {
struct SerializerHeader
{
	char ftype[24];
	uint32_t version_major;
	uint32_t version_minor;

	SerializerHeader()
	{
		std::strncpy( ftype, DATA_HEADER_IDSTRING, sizeof(ftype));
		version_major = ByteOrder<uint32_t>::hton( STRUS_ANALYZER_VERSION_MAJOR);
		version_minor = ByteOrder<uint32_t>::hton( STRUS_ANALYZER_VERSION_MINOR);
	}

	SerializerHeader( const char* content, std::size_t contentsize)
	{
		if (contentsize < sizeof(*this)) throw strus::runtime_error(_TXT("failed to parse header: too small"));
		std::memcpy( this, content, sizeof(*this));
		if (std::strcmp( ftype, DATA_HEADER_IDSTRING) != 0) throw strus::runtime_error(_TXT("failed to parse header: unknown header id string"));
		version_major = ByteOrder<uint32_t>::ntoh( version_major);
		version_minor = ByteOrder<uint32_t>::ntoh( version_minor);
		if (version_major != STRUS_ANALYZER_VERSION_MAJOR) throw strus::runtime_error(_TXT("failed to parse header: incompatible version"));
		if (version_minor < STRUS_ANALYZER_VERSION_MINOR) throw strus::runtime_error(_TXT("failed to parse header: version of data newer than reader"));
	}

	static bool checkType( const char* content, std::size_t contentsize)
	{
		if (contentsize < sizeof(SerializerHeader)) return false;
		if (std::strcmp( content, DATA_HEADER_IDSTRING) != 0) return false;
		return true;
	}

	std::string tostring() const
	{
		return std::string( (const char*)this, sizeof(SerializerHeader));
	}

	static const char* skip( const char* itr)
	{
		return itr + sizeof(SerializerHeader);
	}
};

struct SerializerData
{

	explicit SerializerData( const std::string& filename_)
		:filename(filename_),content()
	{
		SerializerHeader hdr;
		unsigned int ec = writeFile( filename, hdr.tostring());
		if (ec) throw strus::runtime_error(_TXT("error writing pattern matcher serialization to file '%s': %s"), filename.c_str(), ::strerror(ec));
	}

	std::string filename;
	std::string content;

	enum Operation
	{
		TermFeeder_defineLexem,
		TermFeeder_defineSymbol,
		PatternLexer_defineLexem,
		PatternLexer_defineSymbol,
		PatternLexer_compile,
		PatternMatcher_defineTermFrequency,
		PatternMatcher_pushTerm,
		PatternMatcher_pushExpression,
		PatternMatcher_pushPattern,
		PatternMatcher_attachVariable,
		PatternMatcher_definePattern,
		PatternMatcher_compile
	};

	void startCall( const Operation& op)
	{
		pushScalar( (uint8_t)op);
		pushScalar( (uint8_t)op ^ (uint8_t)0xff);
	}
	void pushParam( const std::string& arg)
	{
		content.append( arg);
		content.push_back('\0');
	}
	void pushParam( uint8_t arg)
	{
		pushScalar<int8_t>( arg);
	}
	void pushParam( int arg)
	{
		pushScalar<int32_t>( arg);
	}
	void pushParam( unsigned int arg)
	{
		pushScalar<int32_t>( arg);
	}
	void pushParam( bool arg)
	{
		pushScalar<int8_t>( arg);
	}
	void pushParam( float arg)
	{
		pushScalar<float>( arg);
	}
	void pushParam( double arg)
	{
		pushScalar<double>( arg);
	}
	void endCall()
	{
		content.push_back('\n');
		if (content.size() > PatternSerializer::MaxContentBufSize)
		{
			unsigned int ec = appendFile( filename, content);
			if (ec) throw strus::runtime_error(_TXT("error writing pattern matcher serialization to file '%s': %s"), filename.c_str(), ::strerror(ec));
			content.clear();
		}
	}
	void close()
	{
		unsigned int ec = appendFile( filename, content);
		if (ec) throw strus::runtime_error(_TXT("error writing pattern matcher serialization to file '%s': %s"), filename.c_str(), ::strerror(ec));
	}

private:
	template <typename ScalarType>
	void pushScalar( const ScalarType& val)
	{
		typedef typename ByteOrder<ScalarType>::net_value_type net_value_type;
		net_value_type val_n = ByteOrder<ScalarType>::hton( val);
		content.append( (const char*)&val_n, sizeof(val_n));
	}

private:
	SerializerData( const SerializerData&){}	//... non copyable
	void operator=( const SerializerData&){}	//... non copyable
};
}//namespace

struct Deserializer
{
public:
	explicit Deserializer( InputStream* inputStream_)
		:m_inputStream(inputStream_)
	{
		char buf[ sizeof(SerializerHeader)];
		std::size_t nn = m_inputStream->read( buf, sizeof(buf));
		SerializerHeader hdr( buf, nn);
	}

	std::string readParam_string()
	{
		std::string rt;
		char buf[ 1024];
		std::size_t nn = m_inputStream->readAhead( buf, sizeof(buf));
		for (; nn != 0; nn = m_inputStream->readAhead( buf, sizeof(buf)))
		{
			char const* eos = (const char*)std::memchr( buf, '\0', nn);
			if (eos)
			{
				nn = eos - buf;
				rt.append( buf, nn);
				m_inputStream->read( buf, nn+1);
				return rt;
			}
			else
			{
				rt.append( buf, nn);
			}
			m_inputStream->read( buf, nn);
		}
		unsigned int ec = m_inputStream->error();
		if (ec)
		{
			throw strus::runtime_error(_TXT("error reading deserialization of patterns from file: %s"), ::strerror(ec));
		}
		else
		{
			throw strus::runtime_error(_TXT("unexpected end of stream reading deserialization of patterns from file: %s"));
		}
	}

	SerializerData::Operation startCall()
	{
		uint8_t op = readScalar<uint8_t>();
		uint8_t opinv = readScalar<uint8_t>();
		if (op != (opinv ^ (uint8_t)0xff))
		{
			throw strus::runtime_error(_TXT("corrupt pattern matcher serialization file: check command header"));
		}
		return (SerializerData::Operation)op;
	}
	uint8_t readParam_uint8()
	{
		return readScalar<uint8_t>();
	}
	int readParam_int()
	{
		return readScalar<int32_t>();
	}
	unsigned int readParam_uint()
	{
		return readScalar<uint32_t>();
	}
	bool readParam_bool()
	{
		return (bool)readScalar<uint8_t>();
	}
	float readParam_float()
	{
		return readScalar<float>();
	}
	double readParam_double()
	{
		return readScalar<double>();
	}
	void endCall()
	{
		char eolnmark = readScalar<int8_t>();
		if (eolnmark != '\n')
		{
			throw strus::runtime_error(_TXT("corrupt pattern matcher serialization file: check end of command"));
		}
	}

private:
	template <typename ScalarType>
	ScalarType readScalar()
	{
		typedef typename ByteOrder<ScalarType>::net_value_type net_value_type;
		net_value_type val_n;
		std::size_t nn = m_inputStream->read( (char*)&val_n, sizeof(val_n));
		if (nn < sizeof(val_n))
		{
			unsigned int ec = m_inputStream->error();
			if (ec)
			{
				throw strus::runtime_error(_TXT("error reading deserialization of patterns from file: %s"), ::strerror(ec));
			}
			else
			{
				throw strus::runtime_error(_TXT("unexpected end of stream reading deserialization of patterns from file: %s"));
			}
		}
		return ByteOrder<ScalarType>::ntoh( val_n);
	}

private:
	InputStream* m_inputStream;

private:
	Deserializer( const Deserializer&){}	//... non copyable
	void operator=( const Deserializer&){}	//... non copyable
};


class PatternTermFeederInstance :public PatternTermFeederInstanceInterface
{
public:
	PatternTermFeederInstance( const Reference<SerializerData>& serializerData_, ErrorBufferInterface* errorhnd_)
		:m_errorhnd(errorhnd_),m_serializerData(serializerData_){}
	virtual ~PatternTermFeederInstance(){}

	virtual void defineLexem(
			unsigned int id,
			const std::string& type)
	{
		try
		{
			m_serializerData->startCall( SerializerData::TermFeeder_defineLexem);
			m_serializerData->pushParam( id);
			m_serializerData->pushParam( type);
			m_serializerData->endCall();
		}
		CATCH_ERROR_MAP( _TXT("error in serialize of PatternTermFeeder::defineLexem command: %s"), *m_errorhnd);
	}

	virtual void defineSymbol(
			unsigned int id,
			unsigned int lexemid,
			const std::string& name)
	{
		try
		{
			m_serializerData->startCall( SerializerData::TermFeeder_defineSymbol);
			m_serializerData->pushParam( id);
			m_serializerData->pushParam( lexemid);
			m_serializerData->pushParam( name);
			m_serializerData->endCall();
		}
		CATCH_ERROR_MAP( _TXT("error in serialize of PatternTermFeeder::defineSymbol command: %s"), *m_errorhnd);
	}

	virtual unsigned int getLexem(
			const std::string& ) const
	{
		m_errorhnd->report(_TXT("command PatternTermFeeder::getLexem not implemented in serializer"));
		return 0;
	}

	virtual std::vector<std::string> lexemTypes() const
	{
		m_errorhnd->report(_TXT("command PatternTermFeeder::lexemTypes not implemented in serializer"));
		return std::vector<std::string>();
	}

	virtual unsigned int getSymbol(
			unsigned int ,
			const std::string& ) const
	{
		m_errorhnd->report(_TXT("command PatternTermFeeder::getSymbol not implemented in serializer"));
		return 0;
	}

private:
	ErrorBufferInterface* m_errorhnd;
	Reference<SerializerData> m_serializerData;
};


class PatternLexerInstance :public PatternLexerInstanceInterface
{
public:
	PatternLexerInstance( const Reference<SerializerData>& serializerData_, ErrorBufferInterface* errorhnd_)
		:m_errorhnd(errorhnd_),m_serializerData(serializerData_){}
	virtual ~PatternLexerInstance(){}

	virtual void defineLexem(
			unsigned int id,
			const std::string& expression,
			unsigned int resultIndex,
			unsigned int level,
			analyzer::PositionBind posbind)
	{
		try
		{
			m_serializerData->startCall( SerializerData::PatternLexer_defineLexem);
			m_serializerData->pushParam( id);
			m_serializerData->pushParam( expression);
			m_serializerData->pushParam( resultIndex);
			m_serializerData->pushParam( level);
			m_serializerData->pushParam( (uint8_t)posbind);
			m_serializerData->endCall();
		}
		CATCH_ERROR_MAP( _TXT("error in serialize of PatternLexer::defineLexem command: %s"), *m_errorhnd);
	}

	virtual void defineSymbol(
			unsigned int id,
			unsigned int lexemid,
			const std::string& name)
	{
		try
		{
			m_serializerData->startCall( SerializerData::PatternLexer_defineSymbol);
			m_serializerData->pushParam( id);
			m_serializerData->pushParam( lexemid);
			m_serializerData->pushParam( name);
			m_serializerData->endCall();
		}
		CATCH_ERROR_MAP( _TXT("error in serialize of PatternLexer::defineSymbol command: %s"), *m_errorhnd);
	}

	virtual unsigned int getSymbol(
			unsigned int ,
			const std::string& ) const
	{
		m_errorhnd->report(_TXT("command PatternLexer::getSymbol not implemented in serializer"));
		return 0;
	}

	virtual bool compile( const analyzer::PatternLexerOptions& opt)
	{
		try
		{
			m_serializerData->startCall( SerializerData::PatternLexer_compile);
			m_serializerData->pushParam( (unsigned int)opt.size());
			analyzer::PatternLexerOptions::const_iterator oi = opt.begin(), oe = opt.end();
			for (; oi != oe; ++oi)
			{
				m_serializerData->pushParam( *oi);
			}
			m_serializerData->endCall();
			return true;
		}
		CATCH_ERROR_MAP_RETURN( _TXT("error in serialize of PatternMatcher::compile command: %s"), *m_errorhnd, false);
	}

	virtual PatternLexerContextInterface* createContext() const
	{
		m_errorhnd->report(_TXT("command PatternLexer::createContext not implemented in serializer"));
		return 0;
	}

private:
	ErrorBufferInterface* m_errorhnd;
	Reference<SerializerData> m_serializerData;
};

class PatternMatcherInstance :public PatternMatcherInstanceInterface
{
public:
	PatternMatcherInstance( const Reference<SerializerData>& serializerData_, ErrorBufferInterface* errorhnd_)
		:m_errorhnd(errorhnd_),m_serializerData(serializerData_){}
	virtual ~PatternMatcherInstance(){}

	virtual void defineTermFrequency( unsigned int termid, double df)
	{
		try
		{
			m_serializerData->startCall( SerializerData::PatternMatcher_defineTermFrequency);
			m_serializerData->pushParam( termid);
			m_serializerData->pushParam( df);
			m_serializerData->endCall();
		}
		CATCH_ERROR_MAP( _TXT("error in serialize of PatternMatcher::defineTermFrequency command: %s"), *m_errorhnd);
	}

	virtual void pushTerm( unsigned int termid)
	{
		try
		{
			m_serializerData->startCall( SerializerData::PatternMatcher_pushTerm);
			m_serializerData->pushParam( termid);
			m_serializerData->endCall();
		}
		CATCH_ERROR_MAP( _TXT("error in serialize of PatternMatcher::pushTerm command: %s"), *m_errorhnd);
	}

	virtual void pushExpression(
			JoinOperation operation,
			std::size_t argc, unsigned int range, unsigned int cardinality)
	{
		try
		{
			m_serializerData->startCall( SerializerData::PatternMatcher_pushExpression);
			m_serializerData->pushParam( (uint8_t)operation);
			m_serializerData->pushParam( (unsigned int)argc);
			m_serializerData->pushParam( range);
			m_serializerData->pushParam( cardinality);
			m_serializerData->endCall();
		}
		CATCH_ERROR_MAP( _TXT("error in serialize of PatternMatcher::pushExpression command: %s"), *m_errorhnd);
	}

	virtual void pushPattern( const std::string& name)
	{
		try
		{
			m_serializerData->startCall( SerializerData::PatternMatcher_pushPattern);
			m_serializerData->pushParam( name);
			m_serializerData->endCall();
		}
		CATCH_ERROR_MAP( _TXT("error in serialize of PatternMatcher::pushPattern command: %s"), *m_errorhnd);
	}

	virtual void attachVariable( const std::string& name, float weight)
	{
		try
		{
			m_serializerData->startCall( SerializerData::PatternMatcher_attachVariable);
			m_serializerData->pushParam( name);
			m_serializerData->pushParam( weight);
			m_serializerData->endCall();
		}
		CATCH_ERROR_MAP( _TXT("error in serialize of PatternMatcher::attachVariable command: %s"), *m_errorhnd);
	}

	virtual void definePattern( const std::string& name, bool visible)
	{
		try
		{
			m_serializerData->startCall( SerializerData::PatternMatcher_definePattern);
			m_serializerData->pushParam( name);
			m_serializerData->pushParam( visible);
			m_serializerData->endCall();
		}
		CATCH_ERROR_MAP( _TXT("error in serialize of PatternMatcher::definePattern command: %s"), *m_errorhnd);
	}

	virtual bool compile( const analyzer::PatternMatcherOptions& opt)
	{
		try
		{
			m_serializerData->startCall( SerializerData::PatternMatcher_compile);
			m_serializerData->pushParam( (unsigned int)opt.size());
			analyzer::PatternMatcherOptions::const_iterator oi = opt.begin(), oe = opt.end();
			for (; oi != oe; ++oi)
			{
				m_serializerData->pushParam( oi->first);
				m_serializerData->pushParam( oi->second);
			}
			m_serializerData->endCall();
			return true;
		}
		CATCH_ERROR_MAP_RETURN( _TXT("error in serialize of PatternMatcher::compile command: %s"), *m_errorhnd, false);
	}

	virtual PatternMatcherContextInterface* createContext() const
	{
		m_errorhnd->report(_TXT("command PatternMatcher::createContext not implemented in serializer"));
		return 0;
	}

private:
	ErrorBufferInterface* m_errorhnd;
	Reference<SerializerData> m_serializerData;
};



class PatternTermFeederInstanceText :public PatternTermFeederInstanceInterface
{
public:
	PatternTermFeederInstanceText( std::ostream& output_, ErrorBufferInterface* errorhnd_)
		:m_errorhnd(errorhnd_),m_output(&output_){}
	virtual ~PatternTermFeederInstanceText(){}

	virtual void defineLexem(
			unsigned int id,
			const std::string& type)
	{
		try
		{
			(*m_output) << "defineLexem( " << id << ", " << type << ");" << std::endl;
		}
		CATCH_ERROR_MAP( _TXT("error in serialize of PatternTermFeeder::defineLexem command: %s"), *m_errorhnd);
	}

	virtual void defineSymbol(
			unsigned int id,
			unsigned int lexemid,
			const std::string& name)
	{
		try
		{
			(*m_output) << "defineSymbol( " << id << ", " << lexemid << ", " << name << ");" << std::endl;
		}
		CATCH_ERROR_MAP( _TXT("error in serialize of PatternTermFeeder::defineSymbol command: %s"), *m_errorhnd);
	}

	virtual unsigned int getLexem(
			const std::string& ) const
	{
		m_errorhnd->report(_TXT("command PatternTermFeeder::getLexem not implemented in serializer"));
		return 0;
	}

	virtual std::vector<std::string> lexemTypes() const
	{
		m_errorhnd->report(_TXT("command PatternTermFeeder::lexemTypes not implemented in serializer"));
		return std::vector<std::string>();
	}

	virtual unsigned int getSymbol(
			unsigned int ,
			const std::string& ) const
	{
		m_errorhnd->report(_TXT("command PatternTermFeeder::getSymbol not implemented in serializer"));
		return 0;
	}

private:
	ErrorBufferInterface* m_errorhnd;
	std::ostream* m_output;
};

class PatternLexerInstanceText :public PatternLexerInstanceInterface
{
public:
	PatternLexerInstanceText( std::ostream& output_, ErrorBufferInterface* errorhnd_)
		:m_errorhnd(errorhnd_),m_output(&output_){}
	virtual ~PatternLexerInstanceText(){}

	virtual void defineLexem(
			unsigned int id,
			const std::string& expression,
			unsigned int resultIndex,
			unsigned int level,
			analyzer::PositionBind posbind)
	{
		try
		{
			(*m_output) << "defineLexem( " << id << ", " << expression << ", " << resultIndex << ", " << level << ", " << (unsigned int)(uint8_t)posbind << ");" << std::endl;
		}
		CATCH_ERROR_MAP( _TXT("error in serialize of PatternLexer::defineLexem command: %s"), *m_errorhnd);
	}

	virtual void defineSymbol(
			unsigned int id,
			unsigned int lexemid,
			const std::string& name)
	{
		try
		{
			(*m_output) << "defineSymbol( " << id << ", " << lexemid << ", " << name << ");" << std::endl;
		}
		CATCH_ERROR_MAP( _TXT("error in serialize of PatternLexer::defineSymbol command: %s"), *m_errorhnd);
	}

	virtual unsigned int getSymbol(
			unsigned int ,
			const std::string& ) const
	{
		m_errorhnd->report(_TXT("command PatternLexer::getSymbol not implemented in serializer"));
		return 0;
	}

	virtual bool compile( const analyzer::PatternLexerOptions& opt)
	{
		try
		{
			(*m_output) << "compile( ";
			analyzer::PatternLexerOptions::const_iterator oi = opt.begin(), oe = opt.end();
			for (int oidx=0; oi != oe; ++oi,++oidx)
			{
				if (oidx) (*m_output) << ", ";
				(*m_output) << *oi;
			}
			(*m_output) << ");" << std::endl;
			return true;
		}
		CATCH_ERROR_MAP_RETURN( _TXT("error in serialize of PatternMatcher::compile command: %s"), *m_errorhnd, false);
	}

	virtual PatternLexerContextInterface* createContext() const
	{
		m_errorhnd->report(_TXT("command PatternLexer::createContext not implemented in serializer"));
		return 0;
	}

private:
	ErrorBufferInterface* m_errorhnd;
	std::ostream* m_output;
};

class PatternMatcherInstanceText :public PatternMatcherInstanceInterface
{
public:
	PatternMatcherInstanceText( std::ostream& output_, ErrorBufferInterface* errorhnd_)
		:m_errorhnd(errorhnd_),m_output(&output_){}
	virtual ~PatternMatcherInstanceText(){}

	virtual void defineTermFrequency( unsigned int termid, double df)
	{
		try
		{
			(*m_output) << "defineTermFrequency( " << termid << "," << df << ");" << std::endl;
		}
		CATCH_ERROR_MAP( _TXT("error in serialize of PatternMatcher::defineTermFrequency command: %s"), *m_errorhnd);
	}

	virtual void pushTerm( unsigned int termid)
	{
		try
		{
			(*m_output) << "pushTerm( " << termid << ");" << std::endl;
		}
		CATCH_ERROR_MAP( _TXT("error in serialize of PatternMatcher::pushTerm command: %s"), *m_errorhnd);
	}

	virtual void pushExpression(
			JoinOperation operation,
			std::size_t argc, unsigned int range, unsigned int cardinality)
	{
		try
		{
			(*m_output) << "pushExpression( " << (unsigned int)operation << ", " << argc << ", " << range << ", " << cardinality << ");" << std::endl;
		}
		CATCH_ERROR_MAP( _TXT("error in serialize of PatternMatcher::pushExpression command: %s"), *m_errorhnd);
	}

	virtual void pushPattern( const std::string& name)
	{
		try
		{
			(*m_output) << "pushPattern( " << name << ");" << std::endl;
		}
		CATCH_ERROR_MAP( _TXT("error in serialize of PatternMatcher::pushPattern command: %s"), *m_errorhnd);
	}

	virtual void attachVariable( const std::string& name, float weight)
	{
		try
		{
			(*m_output) << "attachVariable( " << name << ", " << weight << ");" << std::endl;
		}
		CATCH_ERROR_MAP( _TXT("error in serialize of PatternMatcher::attachVariable command: %s"), *m_errorhnd);
	}

	virtual void definePattern( const std::string& name, bool visible)
	{
		try
		{
			(*m_output) << "definePattern( " << name << ", " << (visible?"true":"false") << ");" << std::endl;
		}
		CATCH_ERROR_MAP( _TXT("error in serialize of PatternMatcher::definePattern command: %s"), *m_errorhnd);
	}

	virtual bool compile( const analyzer::PatternMatcherOptions& opt)
	{
		try
		{
			(*m_output) << "compile( ";

			analyzer::PatternMatcherOptions::const_iterator oi = opt.begin(), oe = opt.end();
			for (int oidx=0; oi != oe; ++oi,++oidx)
			{
				if (oidx) (*m_output) << ", ";
				(*m_output) << oi->first << "=" << oi->second;
			}
			(*m_output) << ");" << std::endl;
			return true;
		}
		CATCH_ERROR_MAP_RETURN( _TXT("error in serialize of PatternMatcher::compile command: %s"), *m_errorhnd, false);
	}

	virtual PatternMatcherContextInterface* createContext() const
	{
		m_errorhnd->report(_TXT("command PatternMatcher::createContext not implemented in serializer"));
		return 0;
	}

private:
	ErrorBufferInterface* m_errorhnd;
	std::ostream* m_output;
};


DLL_PUBLIC PatternSerializer::~PatternSerializer()
{
	if (m_lexer) delete m_lexer;
	if (m_feeder) delete m_feeder;
	if (m_matcher) delete m_matcher;
	// m_serializerData is not owned by this
}


DLL_PUBLIC bool PatternSerializer::close()
{
	try
	{
		if (m_serializerData) m_serializerData->close();
		return true;
	}
	CATCH_ERROR_MAP_RETURN( _TXT("cannot close pattern serializer (and flush contents): %s"), *m_errorhnd, false);
}


DLL_PUBLIC PatternSerializer*
	createPatternSerializer(
		const std::string& filename,
		const PatternSerializerType& serializerType,
		ErrorBufferInterface* errorhnd)
{
	try
	{
		if (!g_intl_initialized)
		{
			strus::initMessageTextDomain();
			g_intl_initialized = true;
		}
		Reference<SerializerData> serializerData( new SerializerData( filename));
		switch (serializerType)
		{
			case PatternMatcherWithLexer:
			{
				Reference<PatternLexerInstanceInterface> lexer( new PatternLexerInstance( serializerData, errorhnd));
				if (!lexer.get()) throw strus::runtime_error(_TXT("failed to create pattern lexer instance for serialization"));
				Reference<PatternMatcherInstanceInterface> matcher( new PatternMatcherInstance( serializerData, errorhnd));
				if (!matcher.get()) throw strus::runtime_error(_TXT("failed to create pattern matcher instance for serialization"));

				return new PatternSerializer( serializerData.get(), lexer.release(), matcher.release(), errorhnd);
			}
			break;
			case PatternMatcherWithFeeder:
			{
				Reference<PatternTermFeederInstanceInterface> feeder( new PatternTermFeederInstance( serializerData, errorhnd));
				if (!feeder.get()) throw strus::runtime_error(_TXT("failed to create pattern term feeder instance for serialization"));
				Reference<PatternMatcherInstanceInterface> matcher( new PatternMatcherInstance( serializerData, errorhnd));
				if (!matcher.get()) throw strus::runtime_error(_TXT("failed to create pattern matcher instance for serialization"));

				return new PatternSerializer( serializerData.get(), feeder.release(), matcher.release(), errorhnd);
			}
			break;
		}
		return 0;
	}
	CATCH_ERROR_MAP_RETURN( _TXT("cannot create pattern serializer: %s"), *errorhnd, 0);
}

PatternSerializer*
	createPatternSerializerText(
		std::ostream& output,
		const PatternSerializerType& serializerType,
		ErrorBufferInterface* errorhnd)
{
	try
	{
		if (!g_intl_initialized)
		{
			strus::initMessageTextDomain();
			g_intl_initialized = true;
		}
		switch (serializerType)
		{
			case PatternMatcherWithLexer:
			{
				Reference<PatternLexerInstanceInterface> lexer( new PatternLexerInstanceText( output, errorhnd));
				if (!lexer.get()) throw strus::runtime_error(_TXT("failed to create pattern lexer instance for serialization"));
				Reference<PatternMatcherInstanceInterface> matcher( new PatternMatcherInstanceText( output, errorhnd));
				if (!matcher.get()) throw strus::runtime_error(_TXT("failed to create pattern matcher instance for serialization"));

				return new PatternSerializer( 0 /*serializerData*/, lexer.release(), matcher.release(), errorhnd);
			}
			break;
			case PatternMatcherWithFeeder:
			{
				Reference<PatternTermFeederInstanceInterface> feeder( new PatternTermFeederInstanceText( output, errorhnd));
				if (!feeder.get()) throw strus::runtime_error(_TXT("failed to create pattern term feeder instance for serialization"));
				Reference<PatternMatcherInstanceInterface> matcher( new PatternMatcherInstanceText( output, errorhnd));
				if (!matcher.get()) throw strus::runtime_error(_TXT("failed to create pattern matcher instance for serialization"));

				return new PatternSerializer( 0 /*serializerData*/, feeder.release(), matcher.release(), errorhnd);
			}
			break;
		}
		return 0;
	}
	CATCH_ERROR_MAP_RETURN( _TXT("cannot create pattern serializer to text: %s"), *errorhnd, 0);
}

static void deserializeCommand(
		Deserializer& deserializer,
		PatternLexerInstanceInterface* lexer,
		PatternTermFeederInstanceInterface* feeder,
		PatternMatcherInstanceInterface* matcher,
		ErrorBufferInterface* errorhnd)
{
	SerializerData::Operation op = deserializer.startCall();
	switch (op)
	{
		case SerializerData::TermFeeder_defineLexem:
		{
			if (!feeder) throw strus::runtime_error(_TXT("loading term feeder command when no term feeder defined"));

			unsigned int op_id = deserializer.readParam_uint();
			std::string op_type = deserializer.readParam_string();

			deserializer.endCall();
			feeder->defineLexem( op_id, op_type);
			break;
		}
		case SerializerData::TermFeeder_defineSymbol:
		{
			if (!feeder) throw strus::runtime_error(_TXT("loading term feeder command when no term feeder defined"));
			
			unsigned int op_id = deserializer.readParam_uint();
			unsigned int op_lexemid = deserializer.readParam_uint();
			std::string op_name = deserializer.readParam_string();

			deserializer.endCall();
			feeder->defineSymbol( op_id, op_lexemid, op_name);
			break;
		}
		case SerializerData::PatternLexer_defineLexem:
		{
			if (!lexer) throw strus::runtime_error(_TXT("loading pattern lexer command when no pattern lexer defined"));
			
			unsigned int op_id = deserializer.readParam_uint();
			std::string op_expression = deserializer.readParam_string();
			unsigned int op_resultIndex = deserializer.readParam_uint();
			unsigned int op_level = deserializer.readParam_uint();
			analyzer::PositionBind op_posbind = (analyzer::PositionBind)deserializer.readParam_uint8();
			
			deserializer.endCall();
			lexer->defineLexem( op_id, op_expression, op_resultIndex, op_level, op_posbind);
			break;
		}
		case SerializerData::PatternLexer_defineSymbol:
		{
			if (!lexer) throw strus::runtime_error(_TXT("loading pattern lexer command when no pattern lexer defined"));

			unsigned int op_id = deserializer.readParam_uint();
			unsigned int op_lexemid = deserializer.readParam_uint();
			std::string op_name = deserializer.readParam_string();
			
			deserializer.endCall();
			lexer->defineSymbol( op_id, op_lexemid, op_name);
			break;
		}
		case SerializerData::PatternLexer_compile:
		{
			if (!lexer) throw strus::runtime_error(_TXT("loading pattern lexer command when no pattern lexer defined"));
			unsigned int oi = 0, oe = deserializer.readParam_uint();
			analyzer::PatternLexerOptions opt;
			for (; oi != oe; ++oi)
			{
				std::string elem = deserializer.readParam_string();
				opt( elem);
			}
			deserializer.endCall();
			if (!lexer->compile( opt))
			{
				throw strus::runtime_error(_TXT("error compiling deserialized pattern lexer: %s"), errorhnd->fetchError());
			}
			break;
		}
		case SerializerData::PatternMatcher_defineTermFrequency:
		{
			if (!matcher) throw strus::runtime_error(_TXT("loading pattern matcher command when no pattern matcher defined"));

			unsigned int op_termid = deserializer.readParam_uint();
			double op_df = deserializer.readParam_double();

			deserializer.endCall();
			matcher->defineTermFrequency( op_termid, op_df);
			break;
		}
		case SerializerData::PatternMatcher_pushTerm:
		{
			if (!matcher) throw strus::runtime_error(_TXT("loading pattern matcher command when no pattern matcher defined"));
			unsigned int op_termid = deserializer.readParam_uint();
			deserializer.endCall();
			matcher->pushTerm( op_termid);
			break;
		}
		case SerializerData::PatternMatcher_pushExpression:
		{
			if (!matcher) throw strus::runtime_error(_TXT("loading pattern matcher command when no pattern matcher defined"));

			PatternMatcherInstanceInterface::JoinOperation
				op_operation = (PatternMatcherInstanceInterface::JoinOperation)
					deserializer.readParam_uint8();
			unsigned int op_argc = deserializer.readParam_uint();
			unsigned int op_range = deserializer.readParam_uint();
			unsigned int op_cardinality = deserializer.readParam_uint();

			deserializer.endCall();
			matcher->pushExpression( op_operation, op_argc, op_range, op_cardinality);
			break;
		}
		case SerializerData::PatternMatcher_pushPattern:
		{
			if (!matcher) throw strus::runtime_error(_TXT("loading pattern matcher command when no pattern matcher defined"));
			std::string op_name = deserializer.readParam_string();
			deserializer.endCall();
			matcher->pushPattern( op_name);
			break;
		}
		case SerializerData::PatternMatcher_attachVariable:
		{
			if (!matcher) throw strus::runtime_error(_TXT("loading pattern matcher command when no pattern matcher defined"));

			std::string op_name = deserializer.readParam_string();
			float op_weight = deserializer.readParam_float();

			deserializer.endCall();
			matcher->attachVariable( op_name, op_weight);
			break;
		}
		case SerializerData::PatternMatcher_definePattern:
		{
			if (!matcher) throw strus::runtime_error(_TXT("loading pattern matcher command when no pattern matcher defined"));

			std::string op_name = deserializer.readParam_string();
			bool op_visible = deserializer.readParam_bool();

			deserializer.endCall();
			matcher->definePattern( op_name, op_visible);
			break;
		}
		case SerializerData::PatternMatcher_compile:
		{
			if (!matcher) throw strus::runtime_error(_TXT("loading pattern matcher command when no pattern matcher defined"));
			unsigned int oi = 0, oe = deserializer.readParam_uint();
			analyzer::PatternMatcherOptions opt;
			for (; oi != oe; ++oi)
			{
				std::string elem = deserializer.readParam_string();
				double value = deserializer.readParam_double();
				opt( elem, value);
			}
			deserializer.endCall();
			if (!matcher->compile( opt))
			{
				throw strus::runtime_error(_TXT("error compiling deserialized pattern lexer: %s"), errorhnd->fetchError());
			}
			break;
		}
	}
}

DLL_PUBLIC bool strus::loadPatternMatcherFromSerialization(
		const std::string& filename,
		PatternLexerInstanceInterface* lexer,
		PatternMatcherInstanceInterface* matcher,
		ErrorBufferInterface* errorhnd)
{
	try
	{
		InputStream input( filename);
		Deserializer deserializer( &input);
		while (!input.eof())
		{
			deserializeCommand( deserializer, lexer, 0/*feeder*/, matcher, errorhnd);
		}
		return (!errorhnd->hasError());
	}
	CATCH_ERROR_MAP_RETURN( _TXT("error loading pattern matcher from serialization: %s"), *errorhnd, false);
}

DLL_PUBLIC bool strus::loadPatternMatcherFromSerialization(
		const std::string& filename,
		PatternTermFeederInstanceInterface* feeder,
		PatternMatcherInstanceInterface* matcher,
		ErrorBufferInterface* errorhnd)
{
	try
	{
		InputStream input( filename);
		Deserializer deserializer( &input);
		while (!input.eof())
		{
			deserializeCommand( deserializer, 0/*lexer*/, feeder, matcher, errorhnd);
		}
		return (!errorhnd->hasError());
	}
	CATCH_ERROR_MAP_RETURN( _TXT("error loading pattern matcher from serialization: %s"), *errorhnd, false);
}


DLL_PUBLIC bool strus::isPatternSerializerFile(
		const std::string& filename,
		ErrorBufferInterface* errorhnd)
{
	try
	{
		InputStream input( filename);
		char buf[ sizeof(SerializerHeader)];
		std::size_t nn = input.read( buf, sizeof(buf));
		return SerializerHeader::checkType( buf, nn);
	}
	CATCH_ERROR_MAP_RETURN( _TXT("failed to check file for pattern serialization file format: %s"), *errorhnd, false);
}


