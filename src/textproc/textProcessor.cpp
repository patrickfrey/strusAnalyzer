/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "textProcessor.hpp"
#include "textwolf/charset_utf8.hpp"
#include "textwolf/cstringiterator.hpp"
#include "strus/tokenizerFunctionInterface.hpp"
#include "strus/tokenizerFunctionInstanceInterface.hpp"
#include "strus/tokenizerFunctionContextInterface.hpp"
#include "strus/normalizerFunctionInterface.hpp"
#include "strus/normalizerFunctionInstanceInterface.hpp"
#include "strus/normalizerFunctionContextInterface.hpp"
#include "strus/aggregatorFunctionInterface.hpp"
#include "strus/aggregatorFunctionInstanceInterface.hpp"
#include "strus/analyzer/token.hpp"
#include "strus/documentClassDetectorInterface.hpp"
#include "strus/analyzerErrorBufferInterface.hpp"
#include "strus/lib/detector_std.hpp"
#include "private/utils.hpp"
#include "private/errorUtils.hpp"
#include "private/internationalization.hpp"
#include "resourceDirectory.hpp"
#include <stdexcept>
#include <cstring>

using namespace strus;
using namespace strus::analyzer;

#undef STRUS_LOWLEVEL_DEBUG

TextProcessor::~TextProcessor()
{
	std::map<std::string,TokenizerFunctionInterface*>::iterator ti = m_tokenizer_map.begin(), te = m_tokenizer_map.end();
	for (; ti != te; ++ti)
	{
		delete ti->second;
	}
	std::map<std::string,NormalizerFunctionInterface*>::iterator ni = m_normalizer_map.begin(), ne = m_normalizer_map.end();
	for (; ni != ne; ++ni)
	{
		delete ni->second;
	}
	std::map<std::string,AggregatorFunctionInterface*>::iterator ai = m_aggregator_map.begin(), ae = m_aggregator_map.end();
	for (; ai != ae; ++ai)
	{
		delete ai->second;
	}
	std::vector<DocumentClassDetectorInterface*>::iterator di = m_detectors.begin(), de = m_detectors.end();
	for (; di != de; ++di)
	{
		delete *di;
	}
}

class EmptyNormalizerFunctionContext
	:public NormalizerFunctionContextInterface
{
public:
	explicit EmptyNormalizerFunctionContext( AnalyzerErrorBufferInterface* errorhnd)
		:m_errorhnd(errorhnd){}

	virtual std::string normalize( const char* src, std::size_t srcsize)
	{
		return std::string();
	}
private:
	AnalyzerErrorBufferInterface* m_errorhnd;
};

class EmptyNormalizerInstance
	:public NormalizerFunctionInstanceInterface
{
public:
	explicit EmptyNormalizerInstance( AnalyzerErrorBufferInterface* errorhnd)
		:m_errorhnd(errorhnd){}

	virtual NormalizerFunctionContextInterface* createFunctionContext() const
	{
		
		try
		{
			return new EmptyNormalizerFunctionContext( m_errorhnd);
		}
		CATCH_ERROR_MAP_RETURN( _TXT("error in 'empty' normalizer: %s"), *m_errorhnd, 0);
	}

private:
	AnalyzerErrorBufferInterface* m_errorhnd;
};

class EmptyNormalizerFunction
	:public NormalizerFunctionInterface
{
public:
	explicit EmptyNormalizerFunction( AnalyzerErrorBufferInterface* errorhnd_)
		:m_errorhnd(errorhnd_){}

	virtual NormalizerFunctionInstanceInterface* createInstance( const std::vector<std::string>& args, const TextProcessorInterface*) const
	{
		if (args.size())
		{
			m_errorhnd->report( "no arguments expected for 'empty' normalizer");
			return 0;
		}
		try
		{
			return new EmptyNormalizerInstance( m_errorhnd);
		}
		CATCH_ERROR_MAP_RETURN( _TXT("error in 'empty' normalizer: %s"), *m_errorhnd, 0);
	}

	virtual const char* getDescription() const
	{
		return _TXT("Normalizer mapping input tokens to an empty string");
	}

private:
	AnalyzerErrorBufferInterface* m_errorhnd;
};

class OrigNormalizerFunctionContext
	:public NormalizerFunctionContextInterface
{
public:
	explicit OrigNormalizerFunctionContext( AnalyzerErrorBufferInterface* errorhnd)
		:m_errorhnd(errorhnd){}

	virtual std::string normalize( const char* src, std::size_t srcsize)
	{
		try
		{
			std::string rt;
			std::size_t ii=0;
			for (;ii<srcsize; ++ii)
			{
				if ((unsigned char)src[ii] <= 32)
				{
					for (;ii+1 < srcsize && (unsigned char)src[ii+1] <= 32; ++ii){}
					rt.push_back( ' ');
				}
				else
				{
					rt.push_back( src[ii]);
				}
			}
			return rt;
		}
		CATCH_ERROR_MAP_RETURN( _TXT("error in 'orig' normalizer: %s"), *m_errorhnd, std::string());
	}
private:
	AnalyzerErrorBufferInterface* m_errorhnd;
};

class OrigNormalizerInstance
	:public NormalizerFunctionInstanceInterface
{
public:
	explicit OrigNormalizerInstance( AnalyzerErrorBufferInterface* errorhnd)
		:m_errorhnd(errorhnd){}

	virtual NormalizerFunctionContextInterface* createFunctionContext() const
	{
		try
		{
			return new OrigNormalizerFunctionContext( m_errorhnd);
		}
		CATCH_ERROR_MAP_RETURN( _TXT("error in 'orig' normalizer: %s"), *m_errorhnd, 0);
	}
private:
	AnalyzerErrorBufferInterface* m_errorhnd;
};

class OrigNormalizerFunction
	:public NormalizerFunctionInterface
{
public:
	explicit OrigNormalizerFunction( AnalyzerErrorBufferInterface* errorhnd_)
		:m_errorhnd(errorhnd_){}

	virtual NormalizerFunctionInstanceInterface* createInstance( const std::vector<std::string>& args, const TextProcessorInterface*) const
	{
		if (args.size())
		{
			m_errorhnd->report( "no arguments expected for 'orig' normalizer");
			return 0;
		}
		try
		{
			return new OrigNormalizerInstance( m_errorhnd);
		}
		CATCH_ERROR_MAP_RETURN( _TXT("error in 'orig' normalizer: %s"), *m_errorhnd, 0);
	}

	virtual const char* getDescription() const
	{
		return _TXT("Normalizer mapping the identity of the input tokens");
	}

private:
	AnalyzerErrorBufferInterface* m_errorhnd;
};

class TextNormalizerFunctionContext
	:public NormalizerFunctionContextInterface
{
public:
	explicit TextNormalizerFunctionContext( AnalyzerErrorBufferInterface* errorhnd)
		:m_errorhnd(errorhnd){}

	virtual std::string normalize( const char* src, std::size_t srcsize)
	{
		try
		{
			// [1] Trim input string:
			char const* cc = src;
			for (;(unsigned char)*cc <= 32; ++cc,--srcsize){}
			for (;srcsize > 0 && (unsigned char)cc[srcsize-1] <= 32; --srcsize){}
			if (srcsize == 0) return std::string();

			// [2] Map valid unicode (UTF-8) characters of trimmed input string:
			std::string rt;
			textwolf::charset::UTF8 utf8;
			char buf[16];
			unsigned int bufpos;
			textwolf::CStringIterator itr( cc, srcsize);

			while (*itr)
			{
				bufpos = 0;
				textwolf::UChar value = utf8.value( buf, bufpos, itr);
				if (value != textwolf::charset::UTF8::MaxChar)
				{
					rt.append( buf, bufpos);
				}
				if (value <= 32)
				{
					while (*itr && (unsigned char)*itr <= 32) ++itr;
				}
			}
			return rt;
		}
		CATCH_ERROR_MAP_RETURN( _TXT("error in 'orig' normalizer: %s"), *m_errorhnd, std::string());
	}
private:
	AnalyzerErrorBufferInterface* m_errorhnd;
};

class TextNormalizerInstance
	:public NormalizerFunctionInstanceInterface
{
public:
	explicit TextNormalizerInstance( AnalyzerErrorBufferInterface* errorhnd)
		:m_errorhnd(errorhnd){}

	virtual NormalizerFunctionContextInterface* createFunctionContext() const
	{
		try
		{
			return new TextNormalizerFunctionContext( m_errorhnd);
		}
		CATCH_ERROR_MAP_RETURN( _TXT("error in 'text' normalizer: %s"), *m_errorhnd, 0);
	}
private:
	AnalyzerErrorBufferInterface* m_errorhnd;
};

class TextNormalizerFunction
	:public NormalizerFunctionInterface
{
public:
	explicit TextNormalizerFunction( AnalyzerErrorBufferInterface* errorhnd_)
		:m_errorhnd(errorhnd_){}

	virtual NormalizerFunctionInstanceInterface* createInstance( const std::vector<std::string>& args, const TextProcessorInterface*) const
	{
		if (args.size())
		{
			m_errorhnd->report( "no arguments expected for 'orig' normalizer");
			return 0;
		}
		try
		{
			return new TextNormalizerInstance( m_errorhnd);
		}
		CATCH_ERROR_MAP_RETURN( _TXT("error in 'orig' normalizer: %s"), *m_errorhnd, 0);
	}

	virtual const char* getDescription() const
	{
		return _TXT("Normalizer mapping the identity of the input tokens");
	}

private:
	AnalyzerErrorBufferInterface* m_errorhnd;
};

class ContentTokenizerFunctionContext
	:public TokenizerFunctionContextInterface
{
public:
	explicit ContentTokenizerFunctionContext( AnalyzerErrorBufferInterface* errorhnd)
		:m_errorhnd(errorhnd){}

	virtual std::vector<analyzer::Token>
			tokenize( const char* src, std::size_t srcsize)
	{
		try
		{
			std::vector<analyzer::Token> rt;
			rt.push_back( analyzer::Token( 0, 0, srcsize));
			return rt;
		}
		CATCH_ERROR_MAP_RETURN( _TXT("error in 'content' tokenizer: %s"), *m_errorhnd, std::vector<analyzer::Token>());
	}

private:
	AnalyzerErrorBufferInterface* m_errorhnd;
};

class ContentTokenizerInstance
	:public TokenizerFunctionInstanceInterface
{
public:
	explicit ContentTokenizerInstance( AnalyzerErrorBufferInterface* errorhnd)
		:m_errorhnd(errorhnd){}

	virtual TokenizerFunctionContextInterface* createFunctionContext() const
	{
		try
		{
			return new ContentTokenizerFunctionContext( m_errorhnd);
		}
		CATCH_ERROR_MAP_RETURN( _TXT("error in 'content' tokenizer: %s"), *m_errorhnd, 0);
	}

private:
	AnalyzerErrorBufferInterface* m_errorhnd;
};

class ContentTokenizerFunction
	:public TokenizerFunctionInterface
{
public:
	explicit ContentTokenizerFunction( AnalyzerErrorBufferInterface* errorhnd_)
		:m_errorhnd(errorhnd_){}

	virtual TokenizerFunctionInstanceInterface* createInstance( const std::vector<std::string>& args, const TextProcessorInterface* tp) const
	{
		if (args.size())
		{
			m_errorhnd->report("no arguments expected for 'content' normalizer");
			return 0;
		}
		try
		{
			return new ContentTokenizerInstance( m_errorhnd);
		}
		CATCH_ERROR_MAP_RETURN( _TXT("error in 'content' tokenizer: %s"), *m_errorhnd, 0);
	}

	virtual const char* getDescription() const
	{
		return _TXT("Tokenizer producing one token for each input chunk (identity)");
	}

private:
	AnalyzerErrorBufferInterface* m_errorhnd;
};


class CountAggregatorFunctionInstance
	:public AggregatorFunctionInstanceInterface
{
public:
	/// \brief Constructor
	CountAggregatorFunctionInstance( const std::string& featuretype_, AnalyzerErrorBufferInterface* errorhnd)
		:m_featuretype( utils::tolower( featuretype_)),m_errorhnd(0){}

	virtual double evaluate( const analyzer::Document& document) const
	{
		double rt = 0.0;
		std::vector<Term>::const_iterator
			si = document.searchIndexTerms().begin(),
			se = document.searchIndexTerms().end();

		for (; si != se; ++si)
		{
			if (si->type() == m_featuretype) ++rt;
		}
		return rt;
	}

private:
	std::string m_featuretype;
	AnalyzerErrorBufferInterface* m_errorhnd;
};

class CountAggregatorFunction
	:public AggregatorFunctionInterface
{
public:
	explicit CountAggregatorFunction( AnalyzerErrorBufferInterface* errorhnd_)
		:m_errorhnd(errorhnd_){}

	virtual AggregatorFunctionInstanceInterface* createInstance( const std::vector<std::string>& args) const
	{
		if (args.size() == 0)
		{
			m_errorhnd->report( _TXT("feature type name as argument expected for 'count' aggregator function"));
			return 0;
		}
		if (args.size() > 1)
		{
			m_errorhnd->report( _TXT("too many arguments passed to 'count' aggregator function"));
			return 0;
		}
		try
		{
			return new CountAggregatorFunctionInstance( args[0], m_errorhnd);
		}
		CATCH_ERROR_MAP_RETURN( _TXT("error in 'count' aggregator: %s"), *m_errorhnd, 0);
	}

	virtual const char* getDescription() const
	{
		return _TXT("Aggregator counting the input elements");
	}

private:
	AnalyzerErrorBufferInterface* m_errorhnd;
};

class MaxPosAggregatorFunctionInstance
	:public AggregatorFunctionInstanceInterface
{
public:
	/// \brief Constructor
	MaxPosAggregatorFunctionInstance( const std::string& featuretype_, AnalyzerErrorBufferInterface* errorhnd)
		:m_featuretype( utils::tolower( featuretype_)),m_errorhnd(0){}

	virtual double evaluate( const analyzer::Document& document) const
	{
		unsigned int rt = 0;
		std::vector<Term>::const_iterator
			si = document.searchIndexTerms().begin(),
			se = document.searchIndexTerms().end();

		for (; si != se; ++si)
		{
			if (si->type() == m_featuretype && rt < si->pos()) rt = si->pos();
		}
		return rt;
	}

private:
	std::string m_featuretype;
	AnalyzerErrorBufferInterface* m_errorhnd;
};

class MaxPosAggregatorFunction
	:public AggregatorFunctionInterface
{
public:
	explicit MaxPosAggregatorFunction( AnalyzerErrorBufferInterface* errorhnd_)
		:m_errorhnd(errorhnd_){}

	virtual AggregatorFunctionInstanceInterface* createInstance( const std::vector<std::string>& args) const
	{
		if (args.size() == 0)
		{
			m_errorhnd->report( _TXT("feature type name as argument expected for 'maxpos' aggregator function"));
			return 0;
		}
		if (args.size() > 1)
		{
			m_errorhnd->report( _TXT("too many arguments passed to 'maxpos' aggregator function"));
			return 0;
		}
		try
		{
			return new MaxPosAggregatorFunctionInstance( args[0], m_errorhnd);
		}
		CATCH_ERROR_MAP_RETURN( _TXT("error in 'maxpos' aggregator: %s"), *m_errorhnd, 0);
	}

	virtual const char* getDescription() const
	{
		return _TXT("Aggregator getting the maximum position of the input elements");
	}

private:
	AnalyzerErrorBufferInterface* m_errorhnd;
};

class MinPosAggregatorFunctionInstance
	:public AggregatorFunctionInstanceInterface
{
public:
	/// \brief Constructor
	MinPosAggregatorFunctionInstance( const std::string& featuretype_, AnalyzerErrorBufferInterface* errorhnd)
		:m_featuretype( utils::tolower( featuretype_)),m_errorhnd(0){}

	virtual double evaluate( const analyzer::Document& document) const
	{
		unsigned int rt = 0;
		std::vector<Term>::const_iterator
			si = document.searchIndexTerms().begin(),
			se = document.searchIndexTerms().end();

		for (; si != se; ++si)
		{
			if (si->type() == m_featuretype && (!rt || rt > si->pos())) rt = si->pos();
		}
		return rt;
	}

private:
	std::string m_featuretype;
	AnalyzerErrorBufferInterface* m_errorhnd;
};

class MinPosAggregatorFunction
	:public AggregatorFunctionInterface
{
public:
	explicit MinPosAggregatorFunction( AnalyzerErrorBufferInterface* errorhnd_)
		:m_errorhnd(errorhnd_){}

	virtual AggregatorFunctionInstanceInterface* createInstance( const std::vector<std::string>& args) const
	{
		if (args.size() == 0)
		{
			m_errorhnd->report( _TXT("feature type name as argument expected for 'minpos' aggregator function"));
			return 0;
		}
		if (args.size() > 1)
		{
			m_errorhnd->report( _TXT("too many arguments passed to 'minpos' aggregator function"));
			return 0;
		}
		try
		{
			return new MinPosAggregatorFunctionInstance( args[0], m_errorhnd);
		}
		CATCH_ERROR_MAP_RETURN( _TXT("error in 'minpos' aggregator: %s"), *m_errorhnd, 0);
	}

	virtual const char* getDescription() const
	{
		return _TXT("Aggregator getting the minimum position of the input elements");
	}

private:
	AnalyzerErrorBufferInterface* m_errorhnd;
};

TextProcessor::TextProcessor( AnalyzerErrorBufferInterface* errorhnd)
	:m_errorhnd(errorhnd)
{
	DocumentClassDetectorInterface* dtc;
	if (0==(dtc = createDetector_std( errorhnd))) throw strus::runtime_error(_TXT("error creating text processor"));
	defineDocumentClassDetector( dtc);

	defineTokenizer( "content", new ContentTokenizerFunction( errorhnd));
	defineNormalizer( "orig", new OrigNormalizerFunction(errorhnd));
	defineNormalizer( "text", new TextNormalizerFunction(errorhnd));
	defineNormalizer( "empty", new EmptyNormalizerFunction(errorhnd));
	defineAggregator( "count", new CountAggregatorFunction(errorhnd));
	defineAggregator( "minpos", new MinPosAggregatorFunction(errorhnd));
	defineAggregator( "maxpos", new MaxPosAggregatorFunction(errorhnd));
}

const TokenizerFunctionInterface* TextProcessor::getTokenizer( const std::string& name) const
{
	std::map<std::string,TokenizerFunctionInterface*>::const_iterator
		ti = m_tokenizer_map.find( utils::tolower( name));
	if (ti == m_tokenizer_map.end())
	{
		m_errorhnd->report( _TXT("no tokenizer defined with name '%s'"), name.c_str());
		return 0;
	}
	return ti->second;
}

const NormalizerFunctionInterface* TextProcessor::getNormalizer( const std::string& name) const
{
	std::map<std::string,NormalizerFunctionInterface*>::const_iterator
		ni = m_normalizer_map.find( utils::tolower( name));
	if (ni == m_normalizer_map.end())
	{
		m_errorhnd->report( _TXT("no normalizer defined with name '%s'"), name.c_str());
		return 0;
	}
	return ni->second;
}

const AggregatorFunctionInterface* TextProcessor::getAggregator( const std::string& name) const
{
	std::map<std::string,AggregatorFunctionInterface*>::const_iterator
		ni = m_aggregator_map.find( utils::tolower( name));
	if (ni == m_aggregator_map.end())
	{
		m_errorhnd->report( _TXT("no aggregator function defined with name '%s'"), name.c_str());
		return 0;
	}
	return ni->second;
}


bool TextProcessor::detectDocumentClass( DocumentClass& dclass, const char* contentBegin, std::size_t contentBeginSize) const
{
	std::vector<DocumentClassDetectorInterface*>::const_iterator ci = m_detectors.begin(), ce = m_detectors.end();
	for (; ci != ce; ++ci)
	{
		if ((*ci)->detect( dclass, contentBegin, contentBeginSize))
		{
			return true;
		}
		else
		{
			if (m_errorhnd->hasError()) return false;
		}
	}
	return false;
}

void TextProcessor::defineDocumentClassDetector( DocumentClassDetectorInterface* detector)
{
	try
	{
		m_detectors.push_back( detector);
	}
	catch (const std::bad_alloc&)
	{
		delete detector;
		m_errorhnd->report( _TXT("out of memory"));
	}
}

void TextProcessor::defineTokenizer( const std::string& name, TokenizerFunctionInterface* tokenizer)
{
	try
	{
		std::string id( utils::tolower( name));
		std::map<std::string,TokenizerFunctionInterface*>::iterator ti = m_tokenizer_map.find(id);
		if (ti != m_tokenizer_map.end())
		{
			delete ti->second;
			ti->second = tokenizer;
		}
		else
		{
			m_tokenizer_map[ id] = tokenizer;
		}
	}
	catch (const std::bad_alloc&)
	{
		delete tokenizer;
		m_errorhnd->report( _TXT("out of memory"));
	}
}

void TextProcessor::defineNormalizer( const std::string& name, NormalizerFunctionInterface* normalizer)
{
	try
	{
		std::string id( utils::tolower( name));
		std::map<std::string,NormalizerFunctionInterface*>::iterator ni = m_normalizer_map.find(id);
		if (ni != m_normalizer_map.end())
		{
			delete ni->second;
			ni->second = normalizer;
		}
		else
		{
			m_normalizer_map[ id] = normalizer;
		}
	}
	catch (const std::bad_alloc&)
	{
		delete normalizer;
		m_errorhnd->report( _TXT("out of memory"));
	}
}

void TextProcessor::defineAggregator( const std::string& name, AggregatorFunctionInterface* statfunc)
{
	try
	{
		std::string id( utils::tolower( name));
		std::map<std::string,AggregatorFunctionInterface*>::iterator ai = m_aggregator_map.find(id);
		if (ai != m_aggregator_map.end())
		{
			delete ai->second;
			ai->second = statfunc;
		}
		else
		{
			m_aggregator_map[ id] = statfunc;
		}
	}
	catch (const std::bad_alloc&)
	{
		delete statfunc;
		m_errorhnd->report( _TXT("out of memory"));
	}
}


void TextProcessor::addResourcePath( const std::string& path)
{
	try
	{
		char const* cc = path.c_str();
		char const* ee = std::strchr( cc, STRUS_RESOURCE_PATHSEP);
		for (; ee!=0; cc=ee+1,ee=std::strchr( cc, STRUS_RESOURCE_PATHSEP))
		{
			m_resourcePaths.push_back( utils::trim( std::string( cc, ee)));
#ifdef STRUS_LOWLEVEL_DEBUG
		std::cout << "add resource path '" << m_resourcePaths.back() << "'" << std::endl;
#endif
		}
		m_resourcePaths.push_back( utils::trim( std::string( cc)));
#ifdef STRUS_LOWLEVEL_DEBUG
		std::cout << "add resource path '" << m_resourcePaths.back() << "'" << std::endl;
#endif
	}
	CATCH_ERROR_MAP( _TXT("error in 'TextProcessor::addResourcePath': %s"), *m_errorhnd);
}

std::string TextProcessor::getResourcePath( const std::string& filename) const
{
	try
	{
		std::vector<std::string>::const_iterator
			pi = m_resourcePaths.begin(), pe = m_resourcePaths.end();
		for (; pi != pe; ++pi)
		{
			std::string absfilename( *pi + STRUS_RESOURCE_DIRSEP + filename);
#ifdef STRUS_LOWLEVEL_DEBUG
			std::cout << "check resource path '" << absfilename << "'" << std::endl;
#endif
			if (utils::isFile( absfilename))
			{
				return absfilename;
			}
		}
		throw strus::runtime_error( _TXT("resource file '%s' not found"), filename.c_str());
	}
	CATCH_ERROR_MAP_RETURN( _TXT("error in 'TextProcessor::getResourcePath': %s"), *m_errorhnd, std::string());
}

template <class Map>
static std::vector<std::string> getKeys( const Map& map)
{
	std::vector<std::string> rt;
	typename Map::const_iterator mi = map.begin(), me = map.end();
	for (; mi != me; ++mi)
	{
		rt.push_back( mi->first);
	}
	return rt;
}

template <class Map>
static const char* getDescription_( const Map& map, const std::string& name)
{
	typename Map::const_iterator mi = map.find( utils::tolower( name));
	if (mi != map.end())
	{
		return mi->second->getDescription();
	}
	return 0;
}

const char* TextProcessor::getDescription( FunctionType type, const std::string& name) const
{
	try
	{
		switch (type)
		{
			case TokenizerFunction:
				return getDescription_( m_tokenizer_map, name);
			case NormalizerFunction:
				return getDescription_( m_normalizer_map, name);
			case AggregatorFunction:
				return getDescription_( m_aggregator_map, name);
		}
	}
	catch (const std::bad_alloc&)
	{
		m_errorhnd->report( _TXT("out of memory"));
	}
	return 0;
}

std::vector<std::string> TextProcessor::getFunctionList( TextProcessorInterface::FunctionType type) const
{
	try
	{
		switch (type)
		{
			case TokenizerFunction:
				return getKeys( m_tokenizer_map);
			case NormalizerFunction:
				return getKeys( m_normalizer_map);
			case AggregatorFunction:
				return getKeys( m_aggregator_map);
		}
	}
	catch (const std::bad_alloc&)
	{
		m_errorhnd->report( _TXT("out of memory"));
	}
	return std::vector<std::string>();
}


