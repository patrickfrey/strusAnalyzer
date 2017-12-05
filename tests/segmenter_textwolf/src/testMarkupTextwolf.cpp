/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Test of the document markup of the textwolf based XML segmenter
#include "strus/lib/segmenter_textwolf.hpp"
#include "strus/lib/detector_std.hpp"
#include "strus/lib/error.hpp"
#include "strus/errorBufferInterface.hpp"
#include "strus/segmenterInterface.hpp"
#include "strus/segmenterInstanceInterface.hpp"
#include "strus/segmenterContextInterface.hpp"
#include "strus/segmenterMarkupContextInterface.hpp"
#include "strus/analyzer/documentClass.hpp"
#include "strus/documentClassDetectorInterface.hpp"
#include "strus/base/fileio.hpp"
#include "strus/base/local_ptr.hpp"
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
	std::cerr << "usage: " << argv[0] << " <inpfile> <tagfile> <outfile> <expfile>" << std::endl;
	std::cerr << "<inpfile> = input file" << std::endl;
	std::cerr << "<tagfile> = text file with markups to insert" << std::endl;
	std::cerr << "<outfile> = output file" << std::endl;
	std::cerr << "<expfile> = expected file" << std::endl;
}

static bool isSpace( char ch)
{
	return (ch && (unsigned char)ch <= 32);
}
static bool isDigit( char ch)
{
	return (ch >= '0' && ch <= '9');
}
static bool isAlpha( char ch)
{
	return (((ch|32) >= 'a' && (ch|32) <= 'z') || ch == '_');
}
static bool isAlnum( char ch)
{
	return isAlpha(ch)||isDigit(ch);
}
static std::string parseIdentifier( char const*& si)
{
	std::string rt;
	for (; *si && isAlnum(*si); ++si)
	{
		rt.push_back( *si);
	}
	return rt;
}

static unsigned int parseUintValue( char const*& si)
{
	unsigned int rt = 0, prev = 0;
	for (; *si && !isSpace(*si); ++si)
	{
		if (!isDigit(*si)) throw std::runtime_error( "value is not a non negative integer number");
		rt = (rt * 10) + (*si - '0');
		if (rt < prev) throw std::runtime_error( "integer out of range");
	}
	return rt;
}

static int getLine( char const* si, const char* se)
{
	int rt = 1;
	for (; si != se; ++si)
	{
		if (*si == '\n') ++rt;
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
	while (*si && (unsigned char)*si <= 32 && *si != '\n') ++si;
	return si;
}

struct MarkupDescription
{
	MarkupDescription( unsigned int position_, unsigned int offset_, char type_, const std::string& name_, const std::string& value_)
		:position(position_),offset(offset_),type(type_),name(name_),value(value_){}
	MarkupDescription( const MarkupDescription& o)
		:position(o.position),offset(o.offset),type(o.type),name(o.name),value(o.value){}

	unsigned int position;
	unsigned int offset;
	char type;
	std::string name;
	std::string value;
};

std::vector<MarkupDescription> parseMarkupDescriptions( const std::string& src)
{
	char const* si = src.c_str();
	try
	{
		std::vector<MarkupDescription> rt;
		while (*si)
		{
			si = skipNextNonSpace( si);
			if (!isDigit(*si)) throw std::runtime_error("position number expected as 1st argument of markup description)");
			unsigned int position = parseUintValue( si);
			si = skipNextNonSpace( si);
			unsigned int offset = parseUintValue( si);
			si = skipNextNonSpace( si);
			char type = *si++;
			if (type != '>' && type != '<' && type != '@') throw std::runtime_error("expected '<' or '>' or '@' as type (2nd argument of markup description)");
			si = skipNextNonSpace( si);
			if (!isAlpha(*si)) throw std::runtime_error("identifier expected as 3rd argument of markup description");
			std::string name = parseIdentifier( si);
			si = skipNextNonSpace( si);
			char const* expr = si;
			si = skipEoln( si);
			std::string value( expr, si - expr);
			rt.push_back( MarkupDescription( position, offset, type, name, value));
			++si;
		}
		return rt;
	}
	catch (const std::runtime_error& err)
	{
		std::ostringstream msg;
		msg << "error on line " << getLine( src.c_str(), si) << " of markup description file: " << err.what();
		throw std::runtime_error( msg.str());
	}
}


int main( int argc, const char* argv[])
{
	if (argc <= 1 || std::strcmp( argv[1], "-h") == 0 || std::strcmp( argv[1], "--help") == 0)
	{
		printUsage( argc, argv);
		return 0;
	}
	else if (argc < 5)
	{
		std::cerr << "ERROR too few parameters" << std::endl;
		printUsage( argc, argv);
		return 1;
	}
	else if (argc > 5)
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
		std::string inpfile( argv[1]);
		std::string tagfile( argv[2]);
		std::string outfile( argv[3]);
		std::string expfile( argv[4]);

		strus::local_ptr<strus::SegmenterInterface> segmenter( strus::createSegmenter_textwolf( g_errorhnd));
		if (!segmenter.get()) throw std::runtime_error("failed to create segmenter");
		strus::local_ptr<strus::SegmenterInstanceInterface> segmenterInstance( segmenter->createInstance());
		if (!segmenterInstance.get()) throw std::runtime_error("failed to create segmenter instance");

		std::string tagsrc;
		unsigned int ec;
		ec = strus::readFile( tagfile, tagsrc);
		if (ec) throw std::runtime_error( std::string("error reading markup file ") + tagfile + ": " + ::strerror(ec));
		std::vector<MarkupDescription> markups = parseMarkupDescriptions( tagsrc);

		std::string inpsrc;
		ec = strus::readFile( inpfile, inpsrc);
		if (ec) throw std::runtime_error( std::string("error reading markup file ") + inpfile + ": " + ::strerror(ec));
		strus::local_ptr<strus::DocumentClassDetectorInterface> detector( strus::createDetector_std( g_errorhnd));
		if (!detector.get()) throw std::runtime_error("failed to create document class detector");
		strus::analyzer::DocumentClass dclass;
		if (!detector->detect( dclass, inpsrc.c_str(), inpsrc.size(), true)) throw std::runtime_error("failed to detect document class of input source");

		std::ostringstream outstr;
		strus::local_ptr<strus::SegmenterMarkupContextInterface> segmenterMarkupContext( segmenterInstance->createMarkupContext( dclass, inpsrc));
		if (!segmenterMarkupContext.get()) throw std::runtime_error("failed to create markup segmenter context");
		strus::SegmenterPosition segpos = 0;
		const char* segment;
		std::size_t segmentsize;
		outstr << "Input document:" << std::endl;
		while (segmenterMarkupContext->getNext( segpos, segment, segmentsize))
		{
			outstr << segpos << ": " << segmenterMarkupContext->tagName( segpos) << " " << segmenterMarkupContext->tagLevel( segpos) << std::endl;
		}
		std::vector<MarkupDescription>::const_iterator mi = markups.begin(), me = markups.end();
		for (; mi != me; ++mi)
		{
#ifdef STRUS_LOWLEVEL_DEBUG
			std::cout << "MARKUP " << mi->position << ": " << mi->type << " " << mi->name << "'" << mi->value << "'" << std::endl;
#endif
			if (mi->type == '>')
			{
				segmenterMarkupContext->putOpenTag( mi->position, mi->offset, mi->name);
			}
			else if (mi->type == '<')
			{
				segmenterMarkupContext->putCloseTag( mi->position, mi->offset, mi->name);
			}
			else if (mi->type == '@')
			{
				segmenterMarkupContext->putAttribute( mi->position, mi->offset, mi->name, mi->value);
			}
			else
			{
				throw std::runtime_error("unknown type in markup description");
			}
		}
		outstr << std::endl << "Output document:" << std::endl;
		outstr << segmenterMarkupContext->getContent() << std::endl;

		if (g_errorhnd->hasError())
		{
			throw std::runtime_error( g_errorhnd->fetchError());
		}
		std::cout << outstr.str();

		ec = strus::writeFile( outfile, outstr.str());
		if (ec) throw std::runtime_error( std::string("error writing output file ") + outfile + ": " + ::strerror(ec));

		std::string expsrc;
		ec = strus::readFile( expfile, expsrc);
		if (ec) throw std::runtime_error( std::string("error reading expected file ") + expfile + ": " + ::strerror(ec));

		if (outstr.str() != expsrc)
		{
			throw std::runtime_error("output not as expected");
		}
		std::cerr << "OK" << std::endl;

		delete g_errorhnd;
		return 0;
	}
	catch (const std::bad_alloc&)
	{
		std::ostringstream msg;
		if (g_errorhnd->hasError())
		{
			msg << "(" << g_errorhnd->fetchError() << ")";
		}
		std::cerr << "ERROR memory allocation error" << msg.str() << std::endl;
	}
	catch (const std::runtime_error& e)
	{
		std::ostringstream msg;
		if (g_errorhnd->hasError())
		{
			msg << "(" << g_errorhnd->fetchError() << ")";
		}
		std::cerr << "ERROR " << e.what() << msg.str() << std::endl;
	}
	catch (const std::exception& e)
	{
		std::ostringstream msg;
		if (g_errorhnd->hasError())
		{
			msg << "(" << g_errorhnd->fetchError() << ")";
		}
		std::cerr << "EXCEPTION " << e.what() << msg.str() << std::endl;
	}
	if (g_errorhnd)
	{
		delete g_errorhnd;
	}
	return -1;
}


