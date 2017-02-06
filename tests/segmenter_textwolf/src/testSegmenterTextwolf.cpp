/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Test of the textwolf based XML segmenter
#include "strus/lib/segmenter_textwolf.hpp"
#include "strus/lib/error.hpp"
#include "strus/errorBufferInterface.hpp"
#include "strus/segmenterInterface.hpp"
#include "strus/segmenterInstanceInterface.hpp"
#include "strus/segmenterContextInterface.hpp"
#include "strus/analyzer/documentClass.hpp"
#include "strus/base/fileio.hpp"
#include <memory>
#include <string>
#include <cstring>
#include <stdexcept>
#include <iostream>
#include <sstream>

#undef STRUS_LOWLEVEL_DEBUG

static strus::ErrorBufferInterface* g_errorhnd = 0;

static void printUsage( int argc, const char* argv[])
{
	std::cerr << "usage: " << argv[0] << " <rundir>" << std::endl;
	std::cerr << "<rundir> = directory with the test files rules.txt input.xml expected.txt" << std::endl;
}

static bool isSpace( char ch)
{
	return (ch && (unsigned char)ch <= 32);
}
static unsigned int parseUintValue( char const*& si)
{
	unsigned int rt = 0, prev = 0;
	for (; *si && !isSpace(*si); ++si)
	{
		if (*si < '0' || *si > '9') throw std::runtime_error( "value is not a non negative integer number");
		rt = (rt * 10) + (*si - '0');
		if (rt < prev) throw std::runtime_error( "integer out of range");
	}
	return rt;
}

static const char* skipEoln( char const* si)
{
	while (*si && *si != '\n') ++si;
	return si;
}
static const char* skipNextNonSpace( char const* si)
{
	while (*si && (unsigned char)*si <= 32) ++si;
	return si;
}
static bool isDigit( char ch)
{
	return (ch >= '0' && ch <= '9');
}

struct RuleDescription
{
	RuleDescription( unsigned int startIdx_, const std::string& expression_)
		:startIdx(startIdx_),endIdx(0),expression(expression_){}
	RuleDescription( unsigned int startIdx_, unsigned int endIdx_, const std::string& expression_)
		:startIdx(startIdx_),endIdx(endIdx_),expression(expression_){}
	RuleDescription( const RuleDescription& o)
		:startIdx(o.startIdx),endIdx(o.endIdx),expression(o.expression){}

	unsigned int startIdx;
	unsigned int endIdx;
	std::string expression;
};



std::vector<RuleDescription> parseRuleDescriptions( const std::string& src)
{
	std::vector<RuleDescription> rt;
	char const* si = src.c_str();
	while (*si)
	{
		si = skipNextNonSpace( si);
		if (isDigit(*si))
		{
			unsigned int id = parseUintValue( si);
			unsigned int id2 = 0;
			si = skipNextNonSpace( si);
			if (isDigit(*si))
			{
				id2 = parseUintValue( si);
			}
			si = skipNextNonSpace( si);
			char const* expr = si;
			si = skipEoln( si);
			std::string exprstr( expr, si - expr);
			rt.push_back( RuleDescription( id, id2, exprstr));
		}
		else if (*si == '#')
		{
			si = skipEoln( si);
		}
		else if (*si)
		{
			throw std::runtime_error( "number expected at begin of rule");
		}
	}
	return rt;
}


int main( int argc, const char* argv[])
{
	if (argc <= 1 || std::strcmp( argv[1], "-h") == 0 || std::strcmp( argv[1], "--help") == 0)
	{
		printUsage( argc, argv);
		return 0;
	}
	else if (argc < 2)
	{
		std::cerr << "ERROR too few parameters" << std::endl;
		printUsage( argc, argv);
		return 1;
	}
	else if (argc > 2)
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
		char rulefile[ 256];
		snprintf( rulefile, sizeof(rulefile), "%s%c%s", argv[1], strus::dirSeparator(), "rules.txt");
		char inputfile[ 256];
		snprintf( inputfile, sizeof(inputfile), "%s%c%s", argv[1], strus::dirSeparator(), "input.xml");
		char expectedfile[ 256];
		snprintf( expectedfile, sizeof(expectedfile), "%s%c%s", argv[1], strus::dirSeparator(), "expected.txt");
		char outputfile[ 256];
		snprintf( outputfile, sizeof(outputfile), "%s%c%s", argv[1], strus::dirSeparator(), "output.txt");

		std::auto_ptr<strus::SegmenterInterface> segmenter( strus::createSegmenter_textwolf( g_errorhnd));
		if (!segmenter.get()) throw std::runtime_error("failed to create segmenter");
		std::auto_ptr<strus::SegmenterInstanceInterface> segmenterInstance( segmenter->createInstance());
		if (!segmenterInstance.get()) throw std::runtime_error("failed to create segmenter instance");
		strus::analyzer::DocumentClass dclass( segmenter->mimeType(), "UTF-8");

		std::string rulesrc;
		unsigned int ec = strus::readFile( rulefile, rulesrc);
		if (ec) throw std::runtime_error( std::string("error reading rule file ") + rulefile + ": " + ::strerror(ec));
		std::vector<RuleDescription> rulear = parseRuleDescriptions( rulesrc);
		std::vector<RuleDescription>::const_iterator ri = rulear.begin(), re = rulear.end();
		for (; ri != re; ++ri)
		{
			if (ri->endIdx)
			{
				segmenterInstance->defineSubSection( ri->startIdx, ri->endIdx, ri->expression);
#ifdef STRUS_LOWLEVEL_DEBUG
				std::cout << "SECTION " << ri->startIdx << ":" << ri->endIdx << " " << ri->expression << std::endl;
#endif
			}
			else
			{
				segmenterInstance->defineSelectorExpression( ri->startIdx, ri->expression);
#ifdef STRUS_LOWLEVEL_DEBUG
				std::cout << "MATCH " << ri->startIdx << " " << ri->expression << std::endl;
#endif
			}
		}
		std::auto_ptr<strus::SegmenterContextInterface> segmenterContext( segmenterInstance->createContext( dclass));
		if (!segmenterContext.get()) throw std::runtime_error("failed to create segmenter context");

		std::string inputsrc;
		ec = strus::readFile( inputfile, inputsrc);
		if (ec) throw std::runtime_error( std::string("error reading input file ") + inputfile + ": " + ::strerror(ec));
#ifdef STRUS_LOWLEVEL_DEBUG
		std::cout << "INPUT" << std::endl << inputsrc << std::endl;
#endif
		std::size_t chunksize = 100;
		std::size_t chunkpos = 0;
		for (; chunkpos + chunksize < inputsrc.size(); chunkpos += chunksize)
		{
			segmenterContext->putInput( inputsrc.c_str() + chunkpos, chunksize, false);
#ifdef STRUS_LOWLEVEL_DEBUG
			std::cout << "PUT INPUT" << std::endl << std::string( inputsrc.c_str() + chunkpos, chunksize) << std::endl;
#endif
		}
		chunksize = inputsrc.size() - chunkpos;
		segmenterContext->putInput( inputsrc.c_str() + chunkpos, chunksize, true);
#ifdef STRUS_LOWLEVEL_DEBUG
		std::cout << "PUT INPUT" << std::endl << std::string( inputsrc.c_str() + chunkpos, chunksize) << std::endl;
#endif
		int id = 0;
		strus::SegmenterPosition pos;
		const char* segment;
		std::size_t segmentsize;
		std::ostringstream out;
		while (segmenterContext->getNext( id, pos, segment, segmentsize))
		{
			out << "[" << id << "] " << std::string(segment,segmentsize) << std::endl;
		}
		std::cout << out.str();
		if (g_errorhnd->hasError())
		{
			throw std::runtime_error( g_errorhnd->fetchError());
		}

		ec = strus::writeFile( outputfile, out.str());
		if (ec) throw std::runtime_error( std::string("error writing output file ") + outputfile + ": " + ::strerror(ec));

		std::string expectedsrc;
		ec = strus::readFile( expectedfile, expectedsrc);
		if (ec) throw std::runtime_error( std::string("error reading expected file ") + expectedfile + ": " + ::strerror(ec));

		if (out.str() != expectedsrc)
		{
			throw std::runtime_error("output not as expected");
		}
		std::cerr << "OK" << std::endl;

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


