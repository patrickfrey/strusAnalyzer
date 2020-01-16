/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Test of the textwolf based XML segmenter
#include "strus/lib/doctree.hpp"
#include "strus/lib/error.hpp"
#include "strus/lib/detector_std.hpp"
#include "strus/lib/filelocator.hpp"
#include "strus/lib/textproc.hpp"
#include "strus/base/fileio.hpp"
#include "strus/errorBufferInterface.hpp"
#include "strus/fileLocatorInterface.hpp"
#include "strus/analyzer/documentClass.hpp"
#include "strus/textProcessorInterface.hpp"
#include "strus/documentClassDetectorInterface.hpp"
#include "strus/base/fileio.hpp"
#include "strus/base/local_ptr.hpp"
#include "strus/base/string_format.hpp"
#include <memory>
#include <string>
#include <cstring>
#include <stdexcept>
#include <iostream>
#include <sstream>

#define STRUS_LOWLEVEL_DEBUG

static strus::ErrorBufferInterface* g_errorhnd = 0;

static void printUsage( int argc, const char* argv[])
{
	std::cerr << "usage: " << argv[0] << " <input> <expected> [ <output> ]" << std::endl;
	std::cerr << "<input>    = input file path" << std::endl;
	std::cerr << "<expected> = expected output file path" << std::endl;
	std::cerr << "<output>   = written output file path" << std::endl;
}

bool parseEoln( std::string::const_iterator& ii, std::string::const_iterator ie)
{
	if (ii == ie) return false;
	if (*ii == '\r')
	{
		++ii;
		if (ii != ie && *ii == '\n')
		{
			++ii;
		}
		return true;
	}
	else if (*ii == '\n')
	{
		++ii;
		return true;
	}
	else
	{
		return false;
	}
}

bool testResultExpect( const std::string& result, const std::string& expect)
{
	std::string::const_iterator ri = result.begin(), re = result.end();
	std::string::const_iterator ei = expect.begin(), ee = expect.end();
	while (ri != re && ei != ee)
	{
		bool eoln_e = parseEoln( ei, ee);
		bool eoln_r = parseEoln( ri, re);
		if (eoln_e || eoln_r)
		{
			if (eoln_e && eoln_r) continue; else return false;
		}
		if (*ri != *ei)
		{
			return false;
		}
		++ri;
		++ei;
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
	else if (argc < 3)
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
		const char* inputfilename = argv[1];
		const char* expectfilename = argv[2];
		const char* outputfilename = argv[3];

		strus::local_ptr<strus::FileLocatorInterface> filelocator( strus::createFileLocator_std( g_errorhnd));
		if (!filelocator.get()) throw std::runtime_error( g_errorhnd->fetchError());
		strus::local_ptr<strus::TextProcessorInterface> textproc( strus::createTextProcessor( filelocator.get(), g_errorhnd));
		if (!textproc.get()) throw std::runtime_error( g_errorhnd->fetchError());
		strus::local_ptr<strus::DocumentClassDetectorInterface> classdetector( strus::createDetector_std( textproc.get(), g_errorhnd));
		if (!classdetector.get()) throw std::runtime_error( g_errorhnd->fetchError());

		std::string inputstr;
		std::string expectstr;
		int ec;
		ec = strus::readFile( inputfilename, inputstr);
		if (ec) throw std::runtime_error( strus::string_format( "error reading input file '%s': %s", inputfilename, std::strerror(ec)));
		ec = strus::readFile( expectfilename, expectstr);
		if (ec) throw std::runtime_error( strus::string_format( "error reading expect file '%s': %s", expectfilename, std::strerror(ec)));

		strus::analyzer::DocumentClass dclass;
		if (!classdetector->detect( dclass, inputstr.c_str(), inputstr.size(), true/*isComplete*/))
		{
			throw std::runtime_error( "failed to detect document class");
		}
		strus::local_ptr<strus::DocTree> doctree;
		if (dclass.mimeType() == "application/xml")
		{
			doctree.reset( strus::createDocTree_xml( dclass.encoding().c_str(), inputstr.c_str(), inputstr.size(), g_errorhnd));
		}
		else
		{
			throw std::runtime_error( "only XML documents accepted");
		}
		if (!doctree.get())
		{
			throw std::runtime_error( g_errorhnd->fetchError());
		}
		std::ostringstream outputbuf;
		if (!strus::printDocTree_xml( outputbuf, dclass.encoding().c_str(), *doctree, g_errorhnd))
		{
			throw std::runtime_error( g_errorhnd->fetchError());
		}
		if (g_errorhnd->hasError())
		{
			throw std::runtime_error( g_errorhnd->fetchError());
		}

		std::string outputstr( outputbuf.str());
		if (!testResultExpect( outputstr, expectstr))
		{
			if (outputfilename)
			{
				ec = strus::writeFile( outputfilename, outputstr);
				if (ec) throw std::runtime_error( strus::string_format( "error writing ouptut file '%s': %s", outputfilename, std::strerror(ec)));
			}
			else
			{
				std::cout << outputstr << std::endl;
			}
			throw std::runtime_error("output not as expected");
		}
		else if (outputfilename)
		{
			ec = strus::removeFile( outputfilename, false/*fail_ifnofexist*/);
			if (ec) throw std::runtime_error( strus::string_format( "error deleting ouptut file '%s': %s", outputfilename, std::strerror(ec)));
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


