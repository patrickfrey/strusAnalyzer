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
#include "strus/tokenizerFunctionInstanceInterface.hpp"
#include "strus/tokenizerFunctionInterface.hpp"
#include "strus/base/local_ptr.hpp"
#include "strus/base/fileio.hpp"
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
	std::cerr << "usage: " << argv[0] << " <resourcedir> <textcat config file> <workingdir> <language> <text file to tokenize>" << std::endl;
	std::cerr << "<resourcedir> = location of the resources to load" << std::endl;
	std::cerr << "<textcat config file> = file containing the profile configuration of textcat" << std::endl;
	std::cerr << "<workingdir> = location where the test program is run" << std::endl;
	std::cerr << "<language> = language filter we want to tokenize for" << std::endl;
	std::cerr << "<text file to tokenize> = text file to test tokenizer on" << std::endl;
}

int main( int argc, const char* argv[])
{
	if (argc <= 1 || std::strcmp( argv[1], "-h") == 0 || std::strcmp( argv[1], "--help") == 0)
	{
		printUsage( argc, argv);
		return 0;
	}
	else if (argc < 6)
	{
		std::cerr << "ERROR too few parameters" << std::endl;
		printUsage( argc, argv);
		return 1;
	}
	else if (argc > 6)
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
		
		const char* resourcePath = argv[1];
		const char* textcatConfig = argv[2];
		const char* workingDir = argv[3];
		const char* language = argv[4];
		const char* textFile = argv[5];

		std::string textcatConfigDir;
		std::string textcatConfigFile;

		int ec = strus::getAncestorPath( textcatConfig, 1, textcatConfigDir);
		if (ec) throw std::runtime_error( "failed to get path of text cat config file");
		ec = strus::getFileName( textcatConfig, textcatConfigFile);
		if (ec) throw std::runtime_error( "failed to get base name of text cat config file");

		g_fileLocator->addResourcePath( resourcePath);
		g_fileLocator->addResourcePath( textcatConfigDir);
		
		const strus::TokenizerFunctionInterface* tokenizer = textproc->getTokenizer( "textcat");
		if (!tokenizer)
		{
			throw std::runtime_error( "tokenizer 'textcat' not defined");
		}

		std::vector<std::string> args;
		args.push_back( textcatConfigFile);
		args.push_back( language);
		strus::local_ptr<strus::TokenizerFunctionInstanceInterface> instance( tokenizer->createInstance( args, textproc.get()));
		if (!instance.get())
		{
			throw std::runtime_error( std::string("failed to create tokenizer 'textcat' instance: ") + g_errorhnd->fetchError());
		}	

		std::string s = strus::joinFilePath( workingDir, textFile);
		std::ifstream f( s.c_str());
		if( !f.good()) {
			throw std::runtime_error( "failed to open text file");
		}
		std::string value( (std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
#ifdef STRUS_LOWLEVEL_DEBUG
		std::cout << "input: " << value << std::endl;
#endif
		std::vector<strus::analyzer::Token> result( instance->tokenize( value.c_str(), value.size()));
#ifdef STRUS_LOWLEVEL_DEBUG
		std::cout << "output tokenizer textcat:";
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

		delete g_errorhnd;

		// we expect all tokens to be of the given language
		if (result.size() == 0) {
			return 1;
		}

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
	return 1;
}
