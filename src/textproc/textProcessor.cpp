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
#include "textProcessor.hpp"
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
			m_errorhnd->report( "feature type name as argument expected for 'count' aggregator function");
			return 0;
		}
		if (args.size() > 1)
		{
			m_errorhnd->report( "too many arguments passed to 'count' aggregator function");
			return 0;
		}
		try
		{
			return new CountAggregatorFunctionInstance( args[0], m_errorhnd);
		}
		CATCH_ERROR_MAP_RETURN( _TXT("error in 'count' aggregator: %s"), *m_errorhnd, 0);
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
	defineNormalizer( "empty", new EmptyNormalizerFunction(errorhnd));
	defineAggregator( "count", new CountAggregatorFunction(errorhnd));
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


