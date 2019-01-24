/*
 * Copyright (c) 2016 Andreas Baumann <mail@andreasbaumann.cc>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "strus/errorBufferInterface.hpp"
#include "strus/lib/error.hpp"
#include "strus/lib/textproc.hpp"
#include "strus/lib/filelocator.hpp"
#include "strus/fileLocatorInterface.hpp"
#include "strus/textProcessorInterface.hpp"
#include "private/internationalization.hpp"
#include "strus/base/string_conv.hpp"
#include "strus/base/inputStream.hpp"
#include "strus/base/local_ptr.hpp"
#include "strus/analyzer/documentClass.hpp"

#include <iostream>
#include <cstring>
#include <memory>
#include <vector>
#include <string>
#include <fstream>
#include <streambuf>
#include <stdexcept>

#undef STRUS_LOWLEVEL_DEBUG

static strus::ErrorBufferInterface* g_errorhnd = 0;
static strus::FileLocatorInterface* g_fileLocator = 0;

static void printUsage( int argc, const char* argv[])
{
	std::cerr << "usage: " << argv[0] << " <workingdir> <MIME type> <charset> <file>" << std::endl;
	std::cerr << "<workingdir> = location where the test program is run" << std::endl;
	std::cerr << "<MIME type> = MIME type extected to detect" << std::endl;
	std::cerr << "<charset> = charset expected to detect" << std::endl;
	std::cerr << "<file> = file to test document class detection on" << std::endl;
}

int main( int argc, const char* argv[])
{
	int rt = 0;
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
		g_errorhnd = strus::createErrorBuffer_standard( 0, 2/*threads*/, NULL);
		if (!g_errorhnd) throw std::runtime_error("failed to create error buffer object");
		g_fileLocator = strus::createFileLocator_std( g_errorhnd);
		if (!g_fileLocator) throw std::runtime_error("failed to create file locator");
		strus::local_ptr<strus::TextProcessorInterface> textproc( strus::createTextProcessor( g_fileLocator, g_errorhnd));
		if (!textproc.get()) throw std::runtime_error("failed to create text processor");

		const char* workingDir = argv[1];
		const char* expectedMIMEType = argv[2];
		const char* expectedCharset = argv[3];
		const char* testFile = argv[4];

		g_fileLocator->addResourcePath( workingDir);
				
		std::string s;
		s.append( workingDir);
		s.append( "/");
		s.append( testFile);

		strus::InputStream input( s.c_str());
		char hdrbuf[ 4096];
		std::size_t hdrsize = input.readAhead( hdrbuf, sizeof( hdrbuf));

		strus::analyzer::DocumentClass dclass;
		if (!textproc->detectDocumentClass( dclass, hdrbuf, hdrsize, false))
		{
			throw std::runtime_error( "failed to detect document class"); 
		}

		const char* err = g_errorhnd->fetchError();
		if (err)
		{
			std::cerr << "has error '" << err << "'" << std::endl;
		}

#ifdef STRUS_LOWLEVEL_DEBUG
		std::cout << "MIME type:" << dclass.mimeType() << std::endl;
		std::cout << "charset: " << dclass.encoding() << std::endl;
#endif
		
		if (!strus::caseInsensitiveEquals( dclass.mimeType(), expectedMIMEType)) {
			throw std::runtime_error( "mismatched mime type");
		}
		if (!strus::caseInsensitiveEquals( dclass.encoding(), expectedCharset)) {
			throw std::runtime_error( "mismatched charset");
		}
		rt = 0;
	}
	catch (const std::bad_alloc&)
	{
		std::cerr << "ERROR memory allocation error" << std::endl;
		rt = 2;
	}
	catch (const std::runtime_error& e)
	{
		std::cerr << "ERROR " << e.what() << std::endl;
		rt = 1;
	}
	catch (const std::exception& e)
	{
		std::cerr << "EXCEPTION " << e.what() << std::endl;
		rt = 1;
	}
	if (g_fileLocator)
	{
		delete g_fileLocator;
	}
	if (g_errorhnd)
	{
		delete g_errorhnd;
	}
	return rt;
}
