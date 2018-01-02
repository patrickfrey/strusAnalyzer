/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "strus/lib/aggregator_vsm.hpp"
#include "strus/lib/analyzer.hpp"
#include "strus/lib/detector_std.hpp"
#include "strus/lib/normalizer_charconv.hpp"
#include "strus/lib/normalizer_dateconv.hpp"
#include "strus/lib/normalizer_dictmap.hpp"
#include "strus/lib/normalizer_snowball.hpp"
#include "strus/lib/segmenter_textwolf.hpp"
#include "strus/lib/textproc.hpp"
#include "strus/lib/error.hpp"
#include "strus/lib/tokenizer_punctuation.hpp"
#include "strus/lib/tokenizer_word.hpp"
#include "strus/errorBufferInterface.hpp"
#include "strus/normalizerFunctionInstanceInterface.hpp"
#include "strus/normalizerFunctionInterface.hpp"
#include "strus/segmenterContextInterface.hpp"
#include "strus/segmenterInstanceInterface.hpp"
#include "strus/segmenterInterface.hpp"
#include "strus/textProcessorInterface.hpp"
#include "strus/tokenizerFunctionInstanceInterface.hpp"
#include "strus/tokenizerFunctionInterface.hpp"
#include "strus/analyzer/token.hpp"
#include "strus/base/stdint.h"
#include "strus/base/local_ptr.hpp"
#include "private/internationalization.hpp"
#include "random.hpp"
#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <cstring>
#include <stdexcept>
#include <algorithm>
#include <memory>
#include <cstdarg>
#include <stdio.h>

#undef STRUS_LOWLEVEL_DEBUG

static strus::test::Random g_random;
static strus::ErrorBufferInterface* g_errorhnd = 0;

#ifdef STRUS_LOWLEVEL_DEBUG
static void print( std::ostream& out, const std::string& val)
{
	static const char HEX[] = "0123456789abcdef";
	std::string::const_iterator vi=val.begin(),ve=val.end();
	for (;  vi != ve; ++vi)
	{
		unsigned char lo = (unsigned char)*vi % 16;
		unsigned char hi = (unsigned char)*vi / 16;
		out << HEX[ hi] << HEX[ lo];
	}
}
#endif

static std::string randomString( unsigned int maxSize)
{
	std::string value;
	unsigned int ii=0;
	for (; ii<maxSize; ++ii)
	{
		value.push_back( (char)g_random.get(0,255));
	}
	return value;
}

static void testTokenizer( strus::TextProcessorInterface* textproc, unsigned int testidx, unsigned int maxSize)
{
	static const char* tokenizerdescr[][3] =
	{
		{"punctuation","de",(const char*)0},
		{"punctuation","en",(const char*)0},
		{"content",(const char*)0},
		{"word",(const char*)0},
		{"split",(const char*)0},
		{"regex","[a-zA-Z][a-zA-Z0-9]+\b", (const char*)0},
		{(const char*)0,(const char*)0}
	};

	std::string value = randomString( maxSize);
	std::cout << "running tokenizer test " << testidx << std::endl;

#ifdef STRUS_LOWLEVEL_DEBUG
	std::cout << "input: ";
	print( std::cout, value);
	std::cout << std::endl;
#endif
	std::size_t ti=0;
	for (; tokenizerdescr[ ti][0]; ++ti)
	{
		char const** tk = tokenizerdescr[ ti];
		std::string name( tokenizerdescr[ ti][0]);
		const strus::TokenizerFunctionInterface* tokenizer = textproc->getTokenizer( name);
		if (!tokenizer)
		{
			throw std::runtime_error( std::string("tokenizer '") + name + "' not defined");
		}
		std::vector<std::string> args;
		std::size_t ai=1;
		for (; tk[ai]; ++ai)
		{
			args.push_back( tk[ai]);
		}
		strus::local_ptr<strus::TokenizerFunctionInstanceInterface> instance( tokenizer->createInstance( args, textproc));
		if (!instance.get())
		{
			throw std::runtime_error( std::string("failed to create tokenizer  '") + name + "' instance: " + g_errorhnd->fetchError());
		}
		std::vector<strus::analyzer::Token> result( instance->tokenize( value.c_str(), value.size()));
#ifdef STRUS_LOWLEVEL_DEBUG
		std::cout << "output tokenizer " << name << ":";
		std::vector<strus::analyzer::Token>::const_iterator ri = result.begin(), re = result.end();
		for (; ri != re; ++ri)
		{
			std::cout << " " << ri->strpos << "," << ri->strsize; 
		}
		std::cout << std::endl;
#endif
		const char* err = g_errorhnd->fetchError();
		if (err)
		{
			std::cerr << "has error '" << err << "'" << std::endl;
		}
	}
}

static void testNormalizer( strus::TextProcessorInterface* textproc, unsigned int testidx, unsigned int maxSize)
{
	static const char* normalizerdescr[][4] =
	{
		{"orig",(const char*)0},
		{"text",(const char*)0},
		{"empty",(const char*)0},
		{"stem",(const char*)"de"},
		{"stem",(const char*)"en"},
		{"dictmap","dict.txt",(const char*)0},
		{"lc",(const char*)0},
		{"uc",(const char*)0},
		{"convdia","de",(const char*)0},
		{"convdia","en",(const char*)0},
		{"date2int", "s", "%m/%d/%y", (const char*)0},
		{"ngram",(const char*)0},
		{"regex","([a-zA-Z])[a-zA-Z0-9]+\b", "$1", (const char*)0},
		{(const char*)0,(const char*)0},
	};

	std::string value = randomString( maxSize);
	std::cout << "running normalizer test " << testidx << std::endl;

#ifdef STRUS_LOWLEVEL_DEBUG
	std::cout << "input: ";
	print( std::cout, value);
	std::cout << std::endl;
#endif
	std::size_t ni=0;
	for (; normalizerdescr[ ni][0]; ++ni)
	{
		char const** nrm = normalizerdescr[ ni];
		std::string name( normalizerdescr[ ni][0]);
		const strus::NormalizerFunctionInterface* normalizer = textproc->getNormalizer( name);
		if (!normalizer)
		{
			throw std::runtime_error( std::string("normalizer '") + name + "' not defined");
		}
		std::vector<std::string> args;
		std::size_t ai=1;
		for (; nrm[ai]; ++ai)
		{
			args.push_back( nrm[ai]);
		}
		strus::local_ptr<strus::NormalizerFunctionInstanceInterface> instance( normalizer->createInstance( args, textproc));
		if (!instance.get())
		{
			throw std::runtime_error( std::string("failed to create normalizer '") + name + "' instance: " + g_errorhnd->fetchError());
		}
		std::string result( instance->normalize( value.c_str(), value.size()));
#ifdef STRUS_LOWLEVEL_DEBUG
		std::cout << "output normalizer " << name << ": ";
		print( std::cout, result);
		std::cout << std::endl;
#endif
		const char* err = g_errorhnd->fetchError();
		if (err)
		{
			std::cerr << "has error '" << err << "'" << std::endl;
		}
	}
}

static unsigned int getUintValue( const char* arg)
{
	unsigned int rt = 0, prev = 0;
	char const* cc = arg;
	for (; *cc; ++cc)
	{
		if (*cc < '0' || *cc > '9') throw std::runtime_error( std::string( "parameter is not a non negative integer number: ") + arg);
		rt = (rt * 10) + (*cc - '0');
		if (rt < prev) throw std::runtime_error( std::string( "parameter out of range: ") + arg);
	}
	return rt;
}

static void printUsage( int argc, const char* argv[])
{
	std::cerr << "usage: " << argv[0] << " <nofruns> <maxsize> <resourcedir>" << std::endl;
	std::cerr << "<nofruns> = number of test runs" << std::endl;
	std::cerr << "<maxsize> = maximum size of a string" << std::endl;
	std::cerr << "<resourcedir> = location of the resources to load" << std::endl;
}

int main( int argc, const char* argv[])
{
	if (argc <= 1 || std::strcmp( argv[1], "-h") == 0 || std::strcmp( argv[1], "--help") == 0)
	{
		printUsage( argc, argv);
		return 0;
	}
	else if (argc < 4)
	{
		std::cerr << "ERROR too few parameters" << std::endl;
		printUsage( argc, argv);
		return 1;
	}
	else if (argc > 4)
	{
		std::cerr << "ERROR too many parameters" << std::endl;
		printUsage( argc, argv);
		return 1;
	}
	try
	{
		g_errorhnd = strus::createErrorBuffer_standard( 0, 2);
		if (!g_errorhnd)
		{
			throw std::runtime_error("failed to create error buffer object");
		}
		unsigned int nofRuns = getUintValue( argv[1]);
		unsigned int maxSize = getUintValue( argv[2]);
		const char* resourcePath = argv[3];

		strus::local_ptr<strus::TextProcessorInterface>
			textproc( strus::createTextProcessor( g_errorhnd));
		textproc->addResourcePath( resourcePath);

		unsigned int ri = 0;
		for (; ri < nofRuns; ++ri)
		{
			testTokenizer( textproc.get(), ri, maxSize);
			testNormalizer( textproc.get(), ri, maxSize);
		}
		delete g_errorhnd;
		return 0;
	}
	catch (const std::bad_alloc&)
	{
		std::cerr << "ERROR memory allocation error" << std::endl;
	}
	catch (const std::runtime_error& e)
	{
		std::cerr << "ERROR " << e.what() << std::endl;
	}
	catch (const std::exception& e)
	{
		std::cerr << "EXCEPTION " << e.what() << std::endl;
	}
	if (g_errorhnd)
	{
		delete g_errorhnd;
	}
	return -1;
}


