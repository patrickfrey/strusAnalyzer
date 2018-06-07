/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "textProcessor.hpp"
#include "strus/lib/segmenter_textwolf.hpp"
#include "strus/lib/segmenter_cjson.hpp"
#include "strus/lib/segmenter_tsv.hpp"
#include "strus/lib/segmenter_plain.hpp"
#include "textwolf/charset_utf8.hpp"
#include "textwolf/cstringiterator.hpp"
#include "strus/segmenterInterface.hpp"
#include "strus/tokenizerFunctionInterface.hpp"
#include "strus/tokenizerFunctionInstanceInterface.hpp"
#include "strus/normalizerFunctionInterface.hpp"
#include "strus/normalizerFunctionInstanceInterface.hpp"
#include "strus/aggregatorFunctionInterface.hpp"
#include "strus/aggregatorFunctionInstanceInterface.hpp"
#include "strus/patternLexerInterface.hpp"
#include "strus/patternMatcherInterface.hpp"
#include "strus/patternTermFeederInterface.hpp"
#include "strus/analyzer/token.hpp"
#include "strus/analyzer/functionView.hpp"
#include "strus/documentClassDetectorInterface.hpp"
#include "strus/errorBufferInterface.hpp"
#include "strus/fileLocatorInterface.hpp"
#include "strus/lib/detector_std.hpp"
#include "strus/lib/pattern_termfeeder.hpp"
#include "strus/base/string_conv.hpp"
#include "strus/base/fileio.hpp"
#include "private/errorUtils.hpp"
#include "private/internationalization.hpp"
#include <stdexcept>
#include <cstring>

using namespace strus;
using namespace strus::analyzer;

#undef STRUS_LOWLEVEL_DEBUG

#ifdef STRUS_LOWLEVEL_DEBUG
#include <iostream>
#endif

#define DEFAULT_SEGMENTER "textwolf"

TextProcessor::~TextProcessor()
{
	std::map<std::string,SegmenterInterface*>::iterator si = m_segmenterMap.begin(), se = m_segmenterMap.end();
	for (; si != se; ++si)
	{
		delete si->second;
	}
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
	std::map<std::string,PatternLexerInterface*>::iterator li = m_patternlexer_map.begin(), le = m_patternlexer_map.end();
	for (; li != le; ++li)
	{
		delete li->second;
	}
	std::map<std::string,PatternMatcherInterface*>::iterator mi = m_patternmatcher_map.begin(), me = m_patternmatcher_map.end();
	for (; mi != me; ++mi)
	{
		delete mi->second;
	}
	delete m_patterntermfeeder;
	std::vector<DocumentClassDetectorInterface*>::iterator di = m_detectors.begin(), de = m_detectors.end();
	for (; di != de; ++di)
	{
		delete *di;
	}
}


class EmptyNormalizerInstance
	:public NormalizerFunctionInstanceInterface
{
public:
	explicit EmptyNormalizerInstance( ErrorBufferInterface* errorhnd)
		:m_errorhnd(errorhnd){}

	virtual std::string normalize( const char* src, std::size_t srcsize) const
	{
		return std::string();
	}
	virtual analyzer::FunctionView view() const
	{
		try
		{
			return analyzer::FunctionView( "empty")
			;
		}
		CATCH_ERROR_MAP_RETURN( _TXT("error in introspection: %s"), *m_errorhnd, analyzer::FunctionView());
	}

private:
	ErrorBufferInterface* m_errorhnd;
};

class EmptyNormalizerFunction
	:public NormalizerFunctionInterface
{
public:
	explicit EmptyNormalizerFunction( ErrorBufferInterface* errorhnd_)
		:m_errorhnd(errorhnd_){}

	virtual NormalizerFunctionInstanceInterface* createInstance( const std::vector<std::string>& args, const TextProcessorInterface*) const
	{
		if (args.size())
		{
			m_errorhnd->report( ErrorCodeInvalidArgument, "no arguments expected for 'empty' normalizer");
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
	ErrorBufferInterface* m_errorhnd;
};


class ConstNormalizerInstance
	:public NormalizerFunctionInstanceInterface
{
public:
	explicit ConstNormalizerInstance( const std::string& value_, ErrorBufferInterface* errorhnd)
		:m_errorhnd(errorhnd),m_value(value_){}

	virtual std::string normalize( const char* src, std::size_t srcsize) const
	{
		return m_value;
	}
	virtual analyzer::FunctionView view() const
	{
		try
		{
			return analyzer::FunctionView( "const")
				("value",m_value)
			;
		}
		CATCH_ERROR_MAP_RETURN( _TXT("error in introspection: %s"), *m_errorhnd, analyzer::FunctionView());
	}

private:
	ErrorBufferInterface* m_errorhnd;
	std::string m_value;
};

class ConstNormalizerFunction
	:public NormalizerFunctionInterface
{
public:
	explicit ConstNormalizerFunction( ErrorBufferInterface* errorhnd_)
		:m_errorhnd(errorhnd_){}

	virtual NormalizerFunctionInstanceInterface* createInstance( const std::vector<std::string>& args, const TextProcessorInterface*) const
	{
		if (args.size() != 1)
		{
			m_errorhnd->report( ErrorCodeIncompleteDefinition, "one argument expected for 'const' normalizer");
			return 0;
		}
		try
		{
			return new ConstNormalizerInstance( args[0], m_errorhnd);
		}
		CATCH_ERROR_MAP_RETURN( _TXT("error in 'const' normalizer: %s"), *m_errorhnd, 0);
	}

	virtual const char* getDescription() const
	{
		return _TXT("Normalizer mapping input tokens to a constant string");
	}

private:
	ErrorBufferInterface* m_errorhnd;
};


class OrigNormalizerInstance
	:public NormalizerFunctionInstanceInterface
{
public:
	explicit OrigNormalizerInstance( ErrorBufferInterface* errorhnd)
		:m_errorhnd(errorhnd){}

	virtual std::string normalize( const char* src, std::size_t srcsize) const
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
	virtual analyzer::FunctionView view() const
	{
		try
		{
			return analyzer::FunctionView( "orig")
			;
		}
		CATCH_ERROR_MAP_RETURN( _TXT("error in introspection: %s"), *m_errorhnd, analyzer::FunctionView());
	}

private:
	ErrorBufferInterface* m_errorhnd;
};

class OrigNormalizerFunction
	:public NormalizerFunctionInterface
{
public:
	explicit OrigNormalizerFunction( ErrorBufferInterface* errorhnd_)
		:m_errorhnd(errorhnd_){}

	virtual NormalizerFunctionInstanceInterface* createInstance( const std::vector<std::string>& args, const TextProcessorInterface*) const
	{
		if (args.size())
		{
			m_errorhnd->report( ErrorCodeInvalidArgument, "no arguments expected for 'orig' normalizer");
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
	ErrorBufferInterface* m_errorhnd;
};


class TextNormalizerInstance
	:public NormalizerFunctionInstanceInterface
{
public:
	explicit TextNormalizerInstance( ErrorBufferInterface* errorhnd_)
		:m_errorhnd(errorhnd_){}

	virtual std::string normalize( const char* src, std::size_t srcsize) const
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

	virtual analyzer::FunctionView view() const
	{
		try
		{
			return analyzer::FunctionView( "text")
			;
		}
		CATCH_ERROR_MAP_RETURN( _TXT("error in introspection: %s"), *m_errorhnd, analyzer::FunctionView());
	}

private:
	ErrorBufferInterface* m_errorhnd;
};

class TextNormalizerFunction
	:public NormalizerFunctionInterface
{
public:
	explicit TextNormalizerFunction( ErrorBufferInterface* errorhnd_)
		:m_errorhnd(errorhnd_){}

	virtual NormalizerFunctionInstanceInterface* createInstance( const std::vector<std::string>& args, const TextProcessorInterface*) const
	{
		if (args.size())
		{
			m_errorhnd->report( ErrorCodeInvalidArgument, "no arguments expected for 'orig' normalizer");
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
	ErrorBufferInterface* m_errorhnd;
};


class ContentTokenizerInstance
	:public TokenizerFunctionInstanceInterface
{
public:
	explicit ContentTokenizerInstance( ErrorBufferInterface* errorhnd)
		:m_errorhnd(errorhnd){}

	virtual std::vector<analyzer::Token>
			tokenize( const char* src, std::size_t srcsize) const
	{
		try
		{
			std::vector<analyzer::Token> rt;
			rt.push_back( analyzer::Token( 0/*ord*/, 0/*seg*/, 0/*ofs*/, srcsize));
			return rt;
		}
		CATCH_ERROR_MAP_RETURN( _TXT("error in 'content' tokenizer: %s"), *m_errorhnd, std::vector<analyzer::Token>());
	}

	virtual bool concatBeforeTokenize() const
	{
		return false;
	}

	virtual analyzer::FunctionView view() const
	{
		try
		{
			return analyzer::FunctionView( "content")
			;
		}
		CATCH_ERROR_MAP_RETURN( _TXT("error in introspection: %s"), *m_errorhnd, analyzer::FunctionView());
	}

private:
	ErrorBufferInterface* m_errorhnd;
};

class ContentTokenizerFunction
	:public TokenizerFunctionInterface
{
public:
	explicit ContentTokenizerFunction( ErrorBufferInterface* errorhnd_)
		:m_errorhnd(errorhnd_){}

	virtual TokenizerFunctionInstanceInterface* createInstance( const std::vector<std::string>& args, const TextProcessorInterface* tp) const
	{
		if (args.size())
		{
			m_errorhnd->report( ErrorCodeInvalidArgument, "no arguments expected for 'content' normalizer");
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
	ErrorBufferInterface* m_errorhnd;
};


class CountAggregatorFunctionInstance
	:public AggregatorFunctionInstanceInterface
{
public:
	/// \brief Constructor
	CountAggregatorFunctionInstance( const std::string& featuretype_, ErrorBufferInterface* errorhnd)
		:m_featuretype( string_conv::tolower( featuretype_)),m_errorhnd(0){}

	virtual NumericVariant evaluate( const analyzer::Document& document) const
	{
		unsigned int rt = 0.0;
		std::vector<DocumentTerm>::const_iterator
			si = document.searchIndexTerms().begin(),
			se = document.searchIndexTerms().end();

		for (; si != se; ++si)
		{
			if (si->type() == m_featuretype) ++rt;
		}
		return (NumericVariant::IntType)rt;
	}

	virtual analyzer::FunctionView view() const
	{
		try
		{
			return analyzer::FunctionView( "count")
				("type",m_featuretype)
			;
		}
		CATCH_ERROR_MAP_RETURN( _TXT("error in introspection: %s"), *m_errorhnd, analyzer::FunctionView());
	}

private:
	std::string m_featuretype;
	ErrorBufferInterface* m_errorhnd;
};

class CountAggregatorFunction
	:public AggregatorFunctionInterface
{
public:
	explicit CountAggregatorFunction( ErrorBufferInterface* errorhnd_)
		:m_errorhnd(errorhnd_){}

	virtual AggregatorFunctionInstanceInterface* createInstance( const std::vector<std::string>& args) const
	{
		if (args.size() == 0)
		{
			m_errorhnd->report( ErrorCodeIncompleteDefinition, _TXT("feature type name as argument expected for 'count' aggregator function"));
			return 0;
		}
		if (args.size() > 1)
		{
			m_errorhnd->report( ErrorCodeInvalidArgument, _TXT("too many arguments passed to 'count' aggregator function"));
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
	ErrorBufferInterface* m_errorhnd;
};

class MaxPosAggregatorFunctionInstance
	:public AggregatorFunctionInstanceInterface
{
public:
	/// \brief Constructor
	MaxPosAggregatorFunctionInstance( const std::string& featuretype_, unsigned int incr_, ErrorBufferInterface* errorhnd)
		:m_featuretype( string_conv::tolower( featuretype_)),m_incr(incr_),m_errorhnd(0){}

	virtual NumericVariant evaluate( const analyzer::Document& document) const
	{
		unsigned int rt = 0;
		std::vector<DocumentTerm>::const_iterator
			si = document.searchIndexTerms().begin(),
			se = document.searchIndexTerms().end();

		for (; si != se; ++si)
		{
			if (si->type() == m_featuretype && rt < si->pos()) rt = si->pos();
		}
		return (NumericVariant::IntType)(rt + m_incr);
	}

	virtual analyzer::FunctionView view() const
	{
		try
		{
			return analyzer::FunctionView( "maxpos")
				("type",m_featuretype)
				("incr",m_incr)
			;
		}
		CATCH_ERROR_MAP_RETURN( _TXT("error in introspection: %s"), *m_errorhnd, analyzer::FunctionView());
	}

private:
	std::string m_featuretype;
	unsigned int m_incr;
	ErrorBufferInterface* m_errorhnd;
};

class MaxPosAggregatorFunction
	:public AggregatorFunctionInterface
{
public:
	MaxPosAggregatorFunction( unsigned int incr_, ErrorBufferInterface* errorhnd_)
		:m_incr(incr_),m_errorhnd(errorhnd_){}

	virtual AggregatorFunctionInstanceInterface* createInstance( const std::vector<std::string>& args) const
	{
		if (args.size() == 0)
		{
			m_errorhnd->report( ErrorCodeIncompleteDefinition, _TXT("feature type name as argument expected for 'maxpos' aggregator function"));
			return 0;
		}
		if (args.size() > 1)
		{
			m_errorhnd->report( ErrorCodeInvalidArgument, _TXT("too many arguments passed to 'maxpos' aggregator function"));
			return 0;
		}
		try
		{
			return new MaxPosAggregatorFunctionInstance( args[0], m_incr, m_errorhnd);
		}
		CATCH_ERROR_MAP_RETURN( _TXT("error in 'maxpos' aggregator: %s"), *m_errorhnd, 0);
	}

	virtual const char* getDescription() const
	{
		return _TXT("Aggregator getting the maximum position of the input elements");
	}

private:
	unsigned int m_incr;
	ErrorBufferInterface* m_errorhnd;
};

class MinPosAggregatorFunctionInstance
	:public AggregatorFunctionInstanceInterface
{
public:
	/// \brief Constructor
	MinPosAggregatorFunctionInstance( const std::string& featuretype_, ErrorBufferInterface* errorhnd)
		:m_featuretype( string_conv::tolower( featuretype_)),m_errorhnd(0){}

	virtual NumericVariant evaluate( const analyzer::Document& document) const
	{
		unsigned int rt = 0;
		std::vector<DocumentTerm>::const_iterator
			si = document.searchIndexTerms().begin(),
			se = document.searchIndexTerms().end();

		for (; si != se; ++si)
		{
			if (si->type() == m_featuretype && (!rt || rt > si->pos())) rt = si->pos();
		}
		return (NumericVariant::IntType)rt;
	}

	virtual analyzer::FunctionView view() const
	{
		try
		{
			return analyzer::FunctionView( "minpos")
				("type",m_featuretype)
			;
		}
		CATCH_ERROR_MAP_RETURN( _TXT("error in introspection: %s"), *m_errorhnd, analyzer::FunctionView());
	}

private:
	std::string m_featuretype;
	ErrorBufferInterface* m_errorhnd;
};

class MinPosAggregatorFunction
	:public AggregatorFunctionInterface
{
public:
	explicit MinPosAggregatorFunction( ErrorBufferInterface* errorhnd_)
		:m_errorhnd(errorhnd_){}

	virtual AggregatorFunctionInstanceInterface* createInstance( const std::vector<std::string>& args) const
	{
		if (args.size() == 0)
		{
			m_errorhnd->report( ErrorCodeIncompleteDefinition, _TXT("feature type name as argument expected for 'minpos' aggregator function"));
			return 0;
		}
		if (args.size() > 1)
		{
			m_errorhnd->report( ErrorCodeInvalidArgument, _TXT("too many arguments passed to 'minpos' aggregator function"));
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
	ErrorBufferInterface* m_errorhnd;
};

TextProcessor::TextProcessor( const FileLocatorInterface* filelocator_, ErrorBufferInterface* errorhnd_)
	:m_errorhnd(errorhnd_),m_filelocator(filelocator_),m_patterntermfeeder(createPatternTermFeeder_default(errorhnd_))
{
	if (!m_patterntermfeeder) throw std::runtime_error( _TXT("error creating default pattern term feeder interface for text processor"));
	DocumentClassDetectorInterface* dtc;
	if (0==(dtc = createDetector_std( this, m_errorhnd))) throw std::runtime_error( _TXT("error creating text processor"));
	defineDocumentClassDetector( dtc);

	SegmenterInterface* segref = strus::createSegmenter_textwolf( m_errorhnd);
	if (segref) defineSegmenter( "textwolf", segref);
	segref = strus::createSegmenter_cjson( m_errorhnd);
	if (segref) defineSegmenter( "cjson", segref);
	segref = strus::createSegmenter_tsv( m_errorhnd);
	if (segref) defineSegmenter( "tsv", segref);
	segref = strus::createSegmenter_plain( m_errorhnd);
	if (segref) defineSegmenter( "plain", segref);

	defineTokenizer( "content", new ContentTokenizerFunction( m_errorhnd));
	defineNormalizer( "orig", new OrigNormalizerFunction(m_errorhnd));
	defineNormalizer( "text", new TextNormalizerFunction(m_errorhnd));
	defineNormalizer( "empty", new EmptyNormalizerFunction(m_errorhnd));
	defineNormalizer( "const", new ConstNormalizerFunction(m_errorhnd));
	defineAggregator( "count", new CountAggregatorFunction(m_errorhnd));
	defineAggregator( "minpos", new MinPosAggregatorFunction(m_errorhnd));
	defineAggregator( "maxpos", new MaxPosAggregatorFunction(0,m_errorhnd));
	defineAggregator( "nextpos", new MaxPosAggregatorFunction(1,m_errorhnd));
}

const SegmenterInterface* TextProcessor::getSegmenterByName( const std::string& name) const
{
	try
	{
		std::map<std::string,SegmenterInterface*>::const_iterator ti;
		if (name.empty())
		{
			ti = m_segmenterMap.find( DEFAULT_SEGMENTER);
		}
		else
		{
			ti = m_segmenterMap.find( string_conv::tolower( name));
		}
		if (ti == m_segmenterMap.end())
		{
			if (m_segmenterMap.size() < 8)
			{
				std::string segmenterlist;
				std::map<std::string,SegmenterInterface*>::const_iterator
					si = m_segmenterMap.begin(), se = m_segmenterMap.end();
				for (int sidx=0; si != se; ++si,++sidx)
				{
					if (sidx) segmenterlist.append( ", ");
					segmenterlist.append( si->first);
				}
				m_errorhnd->report( ErrorCodeIncompleteDefinition, _TXT("no segmenter defined with name '%s' (is none of {%s})"), name.c_str(), segmenterlist.c_str());
			}
			else
			{
				m_errorhnd->report( ErrorCodeUnknownIdentifier, _TXT("no segmenter defined with name '%s'"), name.c_str());
			}
			return 0;
		}
		return ti->second;
	}
	catch (const std::bad_alloc&)
	{
		m_errorhnd->report( ErrorCodeOutOfMem, _TXT("out of memory getting segmenter by name '%s'"), name.c_str());
		return 0;
	}
}

const SegmenterInterface* TextProcessor::getSegmenterByMimeType( const std::string& mimetype) const
{
	try
	{
		std::map<std::string,SegmenterInterface*>::const_iterator
			ti = m_mimeSegmenterMap.find( string_conv::tolower( mimetype));
		if (ti == m_mimeSegmenterMap.end())
		{
			if (m_mimeSegmenterMap.size() < 8)
			{
				std::string segmenterlist;
				std::map<std::string,SegmenterInterface*>::const_iterator
					si = m_mimeSegmenterMap.begin(), se = m_mimeSegmenterMap.end();
				for (int sidx=0; si != se; ++si,++sidx)
				{
					if (sidx) segmenterlist.append( ", ");
					segmenterlist.append( si->second->mimeType());
				}
				m_errorhnd->report( ErrorCodeNotImplemented, _TXT("no segmenter defined for MIME type '%s' (is none of {%s})"), mimetype.c_str(), segmenterlist.c_str());
			}
			else
			{
				m_errorhnd->report( ErrorCodeNotImplemented, _TXT("no segmenter defined for MIME type '%s'"), mimetype.c_str());
			}
			return 0;
		}
		return ti->second;
	}
	catch (const std::bad_alloc&)
	{
		m_errorhnd->report( ErrorCodeOutOfMem, _TXT("out of memory getting segmenter by MIME type '%s'"), mimetype.c_str());
		return 0;
	}
}

analyzer::SegmenterOptions TextProcessor::getSegmenterOptions( const std::string& scheme) const
{
	try
	{
		std::map<std::string,analyzer::SegmenterOptions>::const_iterator
			oi = m_schemeSegmenterOptions_map.find( string_conv::tolower(scheme));
		if (oi == m_schemeSegmenterOptions_map.end())
		{
			return analyzer::SegmenterOptions();
		}
		else
		{
			return oi->second;
		}
	}
	catch (const std::bad_alloc&)
	{
		m_errorhnd->report( ErrorCodeOutOfMem, _TXT("out of memory"));
		return analyzer::SegmenterOptions();
	}
}

const TokenizerFunctionInterface* TextProcessor::getTokenizer( const std::string& name) const
{
	try
	{
		std::map<std::string,TokenizerFunctionInterface*>::const_iterator
			ti = m_tokenizer_map.find( string_conv::tolower( name));
		if (ti == m_tokenizer_map.end())
		{
			m_errorhnd->report( ErrorCodeUnknownIdentifier, _TXT("no tokenizer defined with name '%s'"), name.c_str());
			return 0;
		}
		return ti->second;
	}
	catch (const std::bad_alloc&)
	{
		m_errorhnd->report( ErrorCodeOutOfMem, _TXT("out of memory getting tokenizer '%s'"), name.c_str());
		return 0;
	}
}

const NormalizerFunctionInterface* TextProcessor::getNormalizer( const std::string& name) const
{
	try
	{
		std::map<std::string,NormalizerFunctionInterface*>::const_iterator
			ni = m_normalizer_map.find( string_conv::tolower( name));
		if (ni == m_normalizer_map.end())
		{
			m_errorhnd->report( ErrorCodeUnknownIdentifier, _TXT("no normalizer defined with name '%s'"), name.c_str());
			return 0;
		}
		return ni->second;
	}
	catch (const std::bad_alloc&)
	{
		m_errorhnd->report( ErrorCodeOutOfMem, _TXT("out of memory getting normalizer '%s'"), name.c_str());
		return 0;
	}
}

const AggregatorFunctionInterface* TextProcessor::getAggregator( const std::string& name) const
{
	try
	{
		std::map<std::string,AggregatorFunctionInterface*>::const_iterator
			ni = m_aggregator_map.find( string_conv::tolower( name));
		if (ni == m_aggregator_map.end())
		{
			m_errorhnd->report( ErrorCodeUnknownIdentifier, _TXT("no aggregator function defined with name '%s'"), name.c_str());
			return 0;
		}
		return ni->second;
	}
	catch (const std::bad_alloc&)
	{
		m_errorhnd->report( ErrorCodeOutOfMem, _TXT("out of memory getting aggregator '%s'"), name.c_str());
		return 0;
	}
}

const PatternLexerInterface* TextProcessor::getPatternLexer( const std::string& name) const
{
	try
	{
		std::map<std::string,PatternLexerInterface*>::const_iterator
			ni = m_patternlexer_map.find( string_conv::tolower( name));
		if (ni == m_patternlexer_map.end())
		{
			m_errorhnd->report( ErrorCodeUnknownIdentifier, _TXT("no pattern lexer defined with name '%s'"), name.c_str());
			return 0;
		}
		return ni->second;
	}
	catch (const std::bad_alloc&)
	{
		m_errorhnd->report( ErrorCodeOutOfMem, _TXT("out of memory getting pattern lexer '%s'"), name.c_str());
		return 0;
	}
}

const PatternMatcherInterface* TextProcessor::getPatternMatcher( const std::string& name) const
{
	try
	{
		std::map<std::string,PatternMatcherInterface*>::const_iterator
			ni = m_patternmatcher_map.find( string_conv::tolower( name));
		if (ni == m_patternmatcher_map.end())
		{
			m_errorhnd->report( ErrorCodeUnknownIdentifier, _TXT("no pattern matcher defined with name '%s'"), name.c_str());
			return 0;
		}
		return ni->second;
	}
	catch (const std::bad_alloc&)
	{
		m_errorhnd->report( ErrorCodeOutOfMem, _TXT("out of memory getting pattern matcher '%s'"), name.c_str());
		return 0;
	}
}

const PatternTermFeederInterface* TextProcessor::getPatternTermFeeder() const
{
	return m_patterntermfeeder;
}

bool TextProcessor::detectDocumentClass( analyzer::DocumentClass& dclass, const char* contentBegin, std::size_t contentBeginSize, bool isComplete) const
{
	int level = 0;
	std::vector<DocumentClassDetectorInterface*>::const_iterator ci = m_detectors.begin(), ce = m_detectors.end();
	for (; ci != ce; ++ci)
	{
		analyzer::DocumentClass dclass_candidate;
		if ((*ci)->detect( dclass_candidate, contentBegin, contentBeginSize, isComplete))
		{
			if (dclass_candidate.level() >= level)
			{
				dclass = dclass_candidate;
				level = dclass_candidate.level();
			}
		}
		else
		{
			if (m_errorhnd->hasError()) return false;
		}
	}
	return level > 0;
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
		m_errorhnd->report( ErrorCodeOutOfMem, _TXT("out of memory"));
	}
}

void TextProcessor::defineSegmenter( const std::string& name, SegmenterInterface* segmenter)
{
	try
	{
		std::string mimeid( string_conv::tolower(segmenter->mimeType()));
		m_mimeSegmenterMap[ mimeid] = segmenter;
		const char* mimeid2 = std::strchr( mimeid.c_str(), '/');
		if (mimeid2 && m_mimeSegmenterMap.find( mimeid2+1) == m_mimeSegmenterMap.end())
		{
			m_mimeSegmenterMap[ mimeid2+1] = segmenter;
		}
		m_segmenterMap[ string_conv::tolower(name)] = segmenter;
	}
	catch (const std::bad_alloc&)
	{
		delete segmenter;
		m_errorhnd->report( ErrorCodeOutOfMem, _TXT("out of memory"));
	}
}

void TextProcessor::defineSegmenterOptions( const std::string& scheme, const analyzer::SegmenterOptions& options)
{
	try
	{
		m_schemeSegmenterOptions_map[ string_conv::tolower(scheme)] = options;
	}
	catch (const std::bad_alloc&)
	{
		m_errorhnd->report( ErrorCodeOutOfMem, _TXT("out of memory"));
	}
}

void TextProcessor::defineTokenizer( const std::string& name, TokenizerFunctionInterface* tokenizer)
{
	try
	{
		std::string id( string_conv::tolower( name));
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
		m_errorhnd->report( ErrorCodeOutOfMem, _TXT("out of memory"));
	}
}

void TextProcessor::defineNormalizer( const std::string& name, NormalizerFunctionInterface* normalizer)
{
	try
	{
		std::string id( string_conv::tolower( name));
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
		m_errorhnd->report( ErrorCodeOutOfMem, _TXT("out of memory"));
	}
}

void TextProcessor::defineAggregator( const std::string& name, AggregatorFunctionInterface* statfunc)
{
	try
	{
		std::string id( string_conv::tolower( name));
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
		m_errorhnd->report( ErrorCodeOutOfMem, _TXT("out of memory"));
	}
}

void TextProcessor::definePatternLexer( const std::string& name, PatternLexerInterface* func)
{
	try
	{
		std::string id( string_conv::tolower( name));
		std::map<std::string,PatternLexerInterface*>::iterator ai = m_patternlexer_map.find(id);
		if (ai != m_patternlexer_map.end())
		{
			delete ai->second;
			ai->second = func;
		}
		else
		{
			m_patternlexer_map[ id] = func;
		}
	}
	catch (const std::bad_alloc&)
	{
		delete func;
		m_errorhnd->report( ErrorCodeOutOfMem, _TXT("out of memory"));
	}
}

void TextProcessor::definePatternMatcher( const std::string& name, PatternMatcherInterface* func)
{
	try
	{
		std::string id( string_conv::tolower( name));
		std::map<std::string,PatternMatcherInterface*>::iterator ai = m_patternmatcher_map.find(id);
		if (ai != m_patternmatcher_map.end())
		{
			delete ai->second;
			ai->second = func;
		}
		else
		{
			m_patternmatcher_map[ id] = func;
		}
	}
	catch (const std::bad_alloc&)
	{
		delete func;
		m_errorhnd->report( ErrorCodeOutOfMem, _TXT("out of memory"));
	}
}

std::string TextProcessor::getResourceFilePath( const std::string& filename) const
{
	try
	{
		return m_filelocator->getResourceFilePath( filename);
	}
	CATCH_ERROR_MAP_RETURN( _TXT("error in 'TextProcessor::getResourceFilePath': %s"), *m_errorhnd, std::string());
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

std::vector<std::string> TextProcessor::getFunctionList( const FunctionType& type) const
{
	try
	{
		switch (type)
		{
			case Segmenter:
				return getKeys( m_segmenterMap);
			case TokenizerFunction:
				return getKeys( m_tokenizer_map);
			case NormalizerFunction:
				return getKeys( m_normalizer_map);
			case AggregatorFunction:
				return getKeys( m_aggregator_map);
			case PatternLexer:
				return getKeys( m_patternlexer_map);
			case PatternMatcher:
				return getKeys( m_patternmatcher_map);
		}
	}
	catch (const std::bad_alloc&)
	{
		m_errorhnd->report( ErrorCodeOutOfMem, _TXT("out of memory"));
	}
	return std::vector<std::string>();
}


