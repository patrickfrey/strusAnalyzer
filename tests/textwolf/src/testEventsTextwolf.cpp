/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Additional test of the textwolf library
#include "strus/lib/error.hpp"
#include "strus/errorBufferInterface.hpp"
#include "textwolf.hpp"
#include "private/xpathAutomaton.hpp"
#include "strus/base/string_format.hpp"
#include "strus/base/fileio.hpp"
#include "strus/base/local_ptr.hpp"
#include <memory>
#include <string>
#include <cstring>
#include <stdexcept>
#include <iostream>
#include <sstream>


static strus::ErrorBufferInterface* g_errorhnd = 0;

static void printUsage( int argc, const char* argv[])
{
	std::cerr << "usage: " << argv[0] << " <testdir> <execdir>" << std::endl;
	std::cerr << "<testdir> = directory with the test files rules.txt input.xml expected.txt" << std::endl;
	std::cerr << "<execdir> = execution directory where to write output.txt" << std::endl;
}

struct Rule
{
	Rule( const std::string& expression_, const std::string& eventid_, int linecnt_)
		:expression(expression_),eventid(eventid_),linecnt(linecnt_){}
	Rule( const Rule& o)
		:expression(o.expression),eventid(o.eventid),linecnt(o.linecnt){}

	std::string expression;
	std::string eventid;
	int linecnt;
};

static Rule parseRule( const char*& si, int linecnt)
{
	for (; *si && (unsigned char)*si <= 32; ++si){}
	std::string name;
	for (; *si && (unsigned char)*si > 32; ++si)
	{
		name.push_back( *si);
	}
	if ((unsigned char)*si <= 32 && !name.empty())
	{
		for (; *si && *si != '\n' && (unsigned char)*si <= 32; ++si){}
		char const* expression_start = si;
		for (; *si && (unsigned char)*si != '\n'; ++si){}
		return Rule( std::string( expression_start, si-expression_start), name, linecnt);
	}
	else
	{
		throw std::runtime_error("identifier expected as first item of rule definition");
	}
}

static std::vector<Rule> parseRules( const std::string& src)
{
	std::vector<Rule> rt;
	int linecnt = 1;
	char const* si = src.c_str();

	try
	{
		while(*si)
		{
			for (; *si && *si != '\n' && (unsigned char)*si <= 32; ++si){}
			while (*si == '\n')
			{
				++linecnt;
				++si;
				for (; *si && *si != '\n' && (unsigned char)*si <= 32; ++si){}
			}
			if (*si)
			{
				if (*si == '#')
				{
					for (; *si && *si != '\n'; ++si){}
				}
				else
				{
					rt.push_back( parseRule( si, linecnt));
				}
			}
		}
	}
	catch (const std::runtime_error& err)
	{
		throw std::runtime_error( strus::string_format("error in line %d of rule file: %s", linecnt, err.what()));
	}
	return rt;
}

typedef textwolf::XMLPathSelectAutomatonParser<> Automaton;
typedef textwolf::XMLPathSelect<textwolf::charset::UTF8> XMLPathSelect;

static void feedAutomaton( Automaton& automaton, const std::vector<Rule>& rules)
{
	std::vector<Rule>::const_iterator ri = rules.begin(), re = rules.end();
	for (int ridx=0; ri != re; ++ri,++ridx)
	{
		int errorpos = automaton.addExpression( ridx+1, ri->expression.c_str(), ri->expression.size());
		if (errorpos) throw std::runtime_error( strus::string_format("error in selection expression '%s' on line %d, pos %d, at %s", ri->expression.c_str(), ri->linecnt, errorpos, ri->expression.c_str()+errorpos-1));
	}
}

int main( int argc, const char* argv[])
{
	if (argc <= 1 || std::strcmp( argv[1], "-h") == 0 || std::strcmp( argv[1], "--help") == 0)
	{
		printUsage( argc, argv);
		return 0;
	}
	else if (argc < 3)
	{
		std::cerr << "ERROR too few parameters" << std::endl;
		printUsage( argc, argv);
		return 1;
	}
	else if (argc > 3)
	{
		std::cerr << "ERROR too many parameters" << std::endl;
		printUsage( argc, argv);
		return 1;
	}
	try
	{
		g_errorhnd = strus::createErrorBuffer_standard( 0, 2, NULL/*debug trace interface*/);
		if (!g_errorhnd)
		{
			throw std::runtime_error("failed to create error buffer object");
		}
		std::string rulefile = strus::string_format( "%s%c%s", argv[1], strus::dirSeparator(), "rules.txt");
		std::string inputfile = strus::string_format( "%s%c%s", argv[1], strus::dirSeparator(), "input.xml");
		std::string expectedfile = strus::string_format( "%s%c%s", argv[1], strus::dirSeparator(), "expected.txt");
		std::string outputfile = strus::string_format( "%s%c%s", argv[2], strus::dirSeparator(), "output.txt");

		std::string rulesrc;
		int ec = strus::readFile( rulefile, rulesrc);
		if (ec) throw std::runtime_error( std::string("error reading rule file ") + rulefile + ": " + ::strerror(ec));

		std::string inputsrc;
		ec = strus::readFile( inputfile, inputsrc);
		if (ec) throw std::runtime_error( std::string("error reading input file ") + inputfile + ": " + ::strerror(ec));

		std::string expectedsrc;
		ec = strus::readFile( expectedfile, expectedsrc);
		if (ec) throw std::runtime_error( std::string("error reading expected file ") + expectedfile + ": " + ::strerror(ec));

		typedef textwolf::XMLScanner<
				textwolf::SrcIterator,
				textwolf::charset::UTF8,
				textwolf::charset::UTF8,
				std::string
			> XMLScanner;
		Automaton automaton;
		std::vector<Rule> rules = parseRules( rulesrc);
		feedAutomaton( automaton, rules);
		strus::XPathAutomatonContext xpathselect( &automaton);
		textwolf::SrcIterator srciter( inputsrc.c_str(), inputsrc.size(), 0/*no jmpbuf, throws*/);
		XMLScanner scanner( srciter);
		typename XMLScanner::iterator itr = scanner.begin(false);

		std::ostringstream out;
		bool eof = false;
		while (!eof)
		{
			int id;
			while (!xpathselect.getNext( id))
			{
				++itr;

				typename XMLScanner::ElementType et = itr->type();
				if (et == XMLScanner::ErrorOccurred)
				{
					const char* errstr = "";
					scanner.getError( &errstr);
					throw std::runtime_error( strus::string_format( "error in document at position %u: %s", (unsigned int)scanner.getTokenPosition(), errstr));
				}
				else if (et == XMLScanner::Exit)
				{
					eof = true;
					break;
				}
				xpathselect.putElement( itr->type(), itr->content(), itr->size());
			}
			if (!eof)
			{
				int pos = scanner.getTokenPosition();
				std::string segment( itr->content(), itr->size());
	
				out << strus::string_format( "%s [%d]: '%s'", rules[ id-1].eventid.c_str(), pos, segment.c_str()) << std::endl;
			}
		}

		if (out.str() != expectedsrc)
		{
			ec = strus::writeFile( outputfile, out.str());
			if (ec) throw std::runtime_error( std::string("error writing output file ") + outputfile + ": " + ::strerror(ec));
			throw std::runtime_error("output not as expected");
		}
		else
		{
			ec = strus::removeFile( outputfile, false/*fail_ifnofexist*/);
			if (ec) throw std::runtime_error( std::string("error removing file ") + outputfile + ": " + ::strerror(ec));
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


