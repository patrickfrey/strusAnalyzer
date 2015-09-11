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
#include "resourceDirectory.hpp"
#include <stdexcept>
#include <cstring>

using namespace strus;
using namespace strus::analyzer;

#undef STRUS_LOWLEVEL_DEBUG

class EmptyNormalizerFunctionContext
	:public NormalizerFunctionContextInterface
{
public:
	EmptyNormalizerFunctionContext( AnalyzerErrorBufferInterface* errorhnd)
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
		catch (const std::runtime_error& err)
		{
			m_errorhnd->report( std::string( err.what()) + " in 'empty' normalizer");
			return 0;
		}
		catch (const std::bad_alloc&)
		{
			m_errorhnd->report( "out of memory in 'empty' normalizer");
			return 0;
		}
		catch (...)
		{
			m_errorhnd->report( "uncaught exception in 'empty' normalizer");
			return 0;
		}
	}
private:
	AnalyzerErrorBufferInterface* m_errorhnd;
};

class EmptyNormalizerFunction
	:public NormalizerFunctionInterface
{
public:
	virtual NormalizerFunctionInstanceInterface* createInstance( const std::vector<std::string>& args, const TextProcessorInterface*, AnalyzerErrorBufferInterface* errorhnd) const
	{
		if (args.size())
		{
			errorhnd->report( "no arguments expected for 'empty' normalizer");
			return 0;
		}
		try
		{
			return new EmptyNormalizerInstance( errorhnd);
		}
		catch (const std::runtime_error& err)
		{
			errorhnd->report( std::string( err.what()) + " in 'empty' normalizer");
			return 0;
		}
		catch (const std::bad_alloc&)
		{
			errorhnd->report( "out of memory in 'empty' normalizer");
			return 0;
		}
		catch (...)
		{
			errorhnd->report( "uncaught exception in 'empty' normalizer");
			return 0;
		}
	}
};

class OrigNormalizerFunctionContext
	:public NormalizerFunctionContextInterface
{
public:
	OrigNormalizerFunctionContext( AnalyzerErrorBufferInterface* errorhnd)
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
		catch (const std::runtime_error& err)
		{
			m_errorhnd->report( std::string( err.what()) + " in 'orig' normalizer");
			return std::string();
		}
		catch (const std::bad_alloc&)
		{
			m_errorhnd->report( "out of memory in 'orig' normalizer");
			return std::string();
		}
		catch (...)
		{
			m_errorhnd->report( "uncaught exception in 'orig' normalizer");
			return std::string();
		}
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
		catch (const std::runtime_error& err)
		{
			m_errorhnd->report( std::string( err.what()) + " in 'orig' normalizer");
			return 0;
		}
		catch (const std::bad_alloc&)
		{
			m_errorhnd->report( "out of memory in 'orig' normalizer");
			return 0;
		}
		catch (...)
		{
			m_errorhnd->report( "uncaught exception in 'orig' normalizer");
			return 0;
		}
	}
private:
	AnalyzerErrorBufferInterface* m_errorhnd;
};

class OrigNormalizerFunction
	:public NormalizerFunctionInterface
{
public:
	virtual NormalizerFunctionInstanceInterface* createInstance( const std::vector<std::string>& args, const TextProcessorInterface*, AnalyzerErrorBufferInterface* errorhnd) const
	{
		if (args.size())
		{
			errorhnd->report( "no arguments expected for 'orig' normalizer");
			return 0;
		}
		try
		{
			return new OrigNormalizerInstance( errorhnd);
		}
		catch (const std::runtime_error& err)
		{
			errorhnd->report( std::string( err.what()) + " in 'orig' normalizer");
			return 0;
		}
		catch (const std::bad_alloc&)
		{
			errorhnd->report( "out of memory in 'orig' normalizer");
			return 0;
		}
		catch (...)
		{
			errorhnd->report( "uncaught exception in 'orig' normalizer");
			return 0;
		}
	}
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
		catch (const std::runtime_error& err)
		{
			m_errorhnd->report( std::string( err.what()) + " in 'content' tokenizer");
			return std::vector<analyzer::Token>();
		}
		catch (const std::bad_alloc&)
		{
			m_errorhnd->report( "out of memory in 'content' tokenizer");
			return std::vector<analyzer::Token>();
		}
		catch (...)
		{
			m_errorhnd->report( "uncaught exception in 'content' tokenizer");
			return std::vector<analyzer::Token>();
		}
	}

private:
	AnalyzerErrorBufferInterface* m_errorhnd;
};

class ContentTokenizerInstance
	:public TokenizerFunctionInstanceInterface
{
public:
	ContentTokenizerInstance( AnalyzerErrorBufferInterface* errorhnd)
		:m_errorhnd(errorhnd){}

	virtual TokenizerFunctionContextInterface* createFunctionContext() const
	{
		try
		{
			return new ContentTokenizerFunctionContext( m_errorhnd);
		}
		catch (const std::runtime_error& err)
		{
			m_errorhnd->report( std::string( err.what()) + " in 'content' tokenizer");
			return 0;
		}
		catch (const std::bad_alloc&)
		{
			m_errorhnd->report( "out of memory in 'content' tokenizer");
			return 0;
		}
		catch (...)
		{
			m_errorhnd->report( "uncaught exception in 'content' tokenizer");
			return 0;
		}
	}
private:
	AnalyzerErrorBufferInterface* m_errorhnd;
};

class ContentTokenizerFunction
	:public TokenizerFunctionInterface
{
public:
	virtual TokenizerFunctionInstanceInterface* createInstance( const std::vector<std::string>& args, const TextProcessorInterface* tp, AnalyzerErrorBufferInterface* errorhnd) const
	{
		if (args.size())
		{
			errorhnd->report("no arguments expected for 'content' normalizer");
			return 0;
		}
		try
		{
			return new ContentTokenizerInstance( errorhnd);
		}
		catch (const std::runtime_error& err)
		{
			errorhnd->report( std::string( err.what()) + " in 'content' tokenizer");
			return 0;
		}
		catch (const std::bad_alloc&)
		{
			errorhnd->report( "out of memory in 'content' tokenizer");
			return 0;
		}
		catch (...)
		{
			errorhnd->report( "uncaught exception in 'content' tokenizer");
			return 0;
		}
	}
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
	/// \brief Constructor
	CountAggregatorFunction(){}

	virtual AggregatorFunctionInstanceInterface* createInstance( const std::vector<std::string>& args, AnalyzerErrorBufferInterface* errorhnd) const
	{
		if (args.size() == 0)
		{
			errorhnd->report( "feature type name as argument expected for 'count' aggregator function");
			return 0;
		}
		if (args.size() > 1)
		{
			errorhnd->report( "too many arguments passed to 'count' aggregator function");
			return 0;
		}
		try
		{
			return new CountAggregatorFunctionInstance( args[0], errorhnd);
		}
		catch (const std::runtime_error& err)
		{
			errorhnd->report( std::string( err.what()) + " in 'count' aggregator");
			return 0;
		}
		catch (const std::bad_alloc&)
		{
			errorhnd->report( "out of memory in 'count' aggregator");
			return 0;
		}
		catch (...)
		{
			errorhnd->report( "uncaught exception in 'count' aggregator");
			return 0;
		}
	}
};


static ContentTokenizerFunction contentTokenizer;
static OrigNormalizerFunction origNormalizer;
static EmptyNormalizerFunction emptyNormalizer;
static CountAggregatorFunction countAggregator;


TextProcessor::TextProcessor( AnalyzerErrorBufferInterface* errorhnd)
	:m_errorhnd(errorhnd)
{
	defineDocumentClassDetector( getDetector_std());
	defineTokenizer( "content", &contentTokenizer);
	defineNormalizer( "orig", &origNormalizer);
	defineNormalizer( "empty", &emptyNormalizer);
	defineAggregator( "count", &countAggregator);
}

const TokenizerFunctionInterface* TextProcessor::getTokenizer( const std::string& name) const
{
	std::map<std::string,const TokenizerFunctionInterface*>::const_iterator
		ti = m_tokenizer_map.find( utils::tolower( name));
	if (ti == m_tokenizer_map.end())
	{
		m_errorhnd->report( std::string( "no tokenizer defined with name '") + name + "'");
		return 0;
	}
	return ti->second;
}

const NormalizerFunctionInterface* TextProcessor::getNormalizer( const std::string& name) const
{
	std::map<std::string,const NormalizerFunctionInterface*>::const_iterator
		ni = m_normalizer_map.find( utils::tolower( name));
	if (ni == m_normalizer_map.end())
	{
		m_errorhnd->report( std::string( "no normalizer defined with name '") + name + "'");
		return 0;
	}
	return ni->second;
}

const AggregatorFunctionInterface* TextProcessor::getAggregator( const std::string& name) const
{
	std::map<std::string,const AggregatorFunctionInterface*>::const_iterator
		ni = m_aggregator_map.find( utils::tolower( name));
	if (ni == m_aggregator_map.end())
	{
		m_errorhnd->report( std::string( "no aggregator function defined with name '") + name + "'");
		return 0;
	}
	return ni->second;
}


bool TextProcessor::detectDocumentClass( DocumentClass& dclass, const char* contentBegin, std::size_t contentBeginSize) const
{
	std::vector<const DocumentClassDetectorInterface*>::const_iterator ci = m_detectors.begin(), ce = m_detectors.end();
	for (; ci != ce; ++ci)
	{
		if ((*ci)->detect( dclass, contentBegin, contentBeginSize, m_errorhnd))
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

void TextProcessor::defineDocumentClassDetector( const DocumentClassDetectorInterface* detector)
{
	m_detectors.push_back( detector);
}

void TextProcessor::defineTokenizer( const std::string& name, const TokenizerFunctionInterface* tokenizer)
{
	m_tokenizer_map[ utils::tolower( name)] = tokenizer;
}

void TextProcessor::defineNormalizer( const std::string& name, const NormalizerFunctionInterface* normalizer)
{
	m_normalizer_map[ utils::tolower( name)] = normalizer;
}

void TextProcessor::defineAggregator( const std::string& name, const AggregatorFunctionInterface* statfunc)
{
	m_aggregator_map[ utils::tolower( name)] = statfunc;
}


void TextProcessor::addResourcePath( const std::string& path)
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

std::string TextProcessor::getResourcePath( const std::string& filename) const
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
	throw std::runtime_error( std::string( "resource file '") + filename + "' not found");
}


