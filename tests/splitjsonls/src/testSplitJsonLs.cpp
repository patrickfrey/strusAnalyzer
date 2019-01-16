/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Exported functions of the strus segmenter library
#include "strus/lib/segmenter_cjson.hpp"
#include "strus/lib/error.hpp"
#include "strus/errorBufferInterface.hpp"
#include "strus/analyzer/documentClass.hpp"
#include "strus/base/fileio.hpp"
#include "strus/base/local_ptr.hpp"
#include "strus/base/string_format.hpp"
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
	std::cerr << "usage: " << argv[0] << " <rundir> <inputfile> <expectfile>" << std::endl;
	std::cerr << "<rundir> = directory with the test files" << std::endl;
	std::cerr << "<inputfile> = JSON list file to process" << std::endl;
	std::cerr << "<expectfile> = file containing the expected result" << std::endl;
}

static bool compareResult( const std::string& res, const std::string& exp)
{
	std::string::const_iterator ri = res.begin(), re = res.end();
	std::string::const_iterator ei = exp.begin(), ee = exp.end();
	while (ri != re && ei != ee)
	{
		if ((*ri == '\r' || *ri == '\n') && (*ei == '\r' || *ei == '\n'))
		{
			while (ri != re && (*ri == '\r' || *ri == '\n')) ++ri;
			while (ei != ee && (*ei == '\r' || *ei == '\n')) ++ei;
		}
		else if (*ri == *ei)
		{
			++ri;
			++ei;
		}
		else
		{
			return false;
		}
	}
	return (ri == re && ei == ee);
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
		g_errorhnd = strus::createErrorBuffer_standard( 0, 2, NULL/*debug trace interface*/);
		if (!g_errorhnd)
		{
			throw std::runtime_error("failed to create error buffer object");
		}
		std::string execdir = argv[ 1];
		std::string inputfilename = argv[ 2];
		std::string expectfilename = argv[ 3];
		std::string outputfilename = strus::joinFilePath( execdir, "RES");

		strus::analyzer::DocumentClass dclass( "application/json", "UTF-8");
		std::string inputsrc;
		int ec = strus::readFile( inputfilename, inputsrc);
		if (ec) throw std::runtime_error( strus::string_format("error reading input file '%s': %s", inputfilename.c_str(), ::strerror(ec)));

		std::vector<std::string> resultlist = strus::splitJsonDocumentList( dclass.encoding(), inputsrc, g_errorhnd);
		if (g_errorhnd->hasError())
		{
			throw std::runtime_error( strus::string_format( "error parsing json list file: %s", g_errorhnd->fetchError()));
		}
		std::cerr << strus::string_format( "split input into %d documents", (int)resultlist.size()) << std::endl;
		std::ostringstream resultbuf;
		
		std::vector<std::string>::const_iterator ri = resultlist.begin(), re = resultlist.end();
		for (int ridx=1; ri != re; ++ri,++ridx)
		{
			resultbuf << strus::string_format( "## DOC %d:", ridx) << std::endl;
			resultbuf << *ri << std::endl;
		}
		std::string result = resultbuf.str();

		ec = strus::writeFile( outputfilename, result);
		if (ec) throw std::runtime_error( strus::string_format("error writing output file '%s': %s", outputfilename.c_str(), ::strerror(ec)));

		std::string expected;
		ec = strus::readFile( expectfilename, expected);
		if (ec) throw std::runtime_error( strus::string_format("error reading expected file '%s': %s", expectfilename.c_str(), ::strerror(ec)));

		if (!compareResult( result, expected))
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

