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
#include "strus/lib/contentstats_std.hpp"
#include "strus/lib/detector_std.hpp"
#include "strus/contentStatisticsInterface.hpp"
#include "strus/contentStatisticsContextInterface.hpp"
#include "strus/documentClassDetectorInterface.hpp"
#include "strus/tokenizerFunctionInterface.hpp"
#include "strus/tokenizerFunctionInstanceInterface.hpp"
#include "strus/normalizerFunctionInterface.hpp"
#include "strus/normalizerFunctionInstanceInterface.hpp"
#include "strus/textProcessorInterface.hpp"
#include "strus/analyzer/documentClass.hpp"
#include "strus/analyzer/contentStatisticsItem.hpp"
#include "private/internationalization.hpp"
#include "strus/base/string_conv.hpp"
#include "strus/base/inputStream.hpp"
#include "strus/base/local_ptr.hpp"
#include "strus/base/fileio.hpp"

#include <iostream>
#include <sstream>
#include <cstring>
#include <memory>
#include <vector>
#include <string>
#include <stdexcept>

#undef STRUS_LOWLEVEL_DEBUG

static strus::ErrorBufferInterface* g_errorhnd = 0;

static void printUsage( int argc, const char* argv[])
{
	std::cerr << "usage: " << argv[0] << " <datadir> <MIME type> <charset>" << std::endl;
	std::cerr << "<datadir> = location where the test data is found" << std::endl;
	std::cerr << "<MIME type> = MIME type of the input files" << std::endl;
	std::cerr << "<charset> = charset of the input files" << std::endl;
}

struct TokenizerDef
{
	const char* name;
	const char* arg[10];
};
struct NormalizerDef
{
	const char* name;
	const char* arg[8];
};
struct LibraryElement
{
	const char* type;
	const char* regex;
	int priority;
	int minLength;
	int maxLength;
	TokenizerDef tokenizer;
	NormalizerDef normalizers[ 8];
};
static const LibraryElement g_testLibrary[32] =
{
	{"title",".*",1,1,16,{"word",{NULL}},{{"orig",{NULL}},{NULL}}},
	{"text",".*",1,1,-1,{"word",{NULL}},{{"orig",{NULL}},{NULL}}},
	{"id","[0-9]+",2,1,1,{"word",{NULL}},{{"orig",{NULL}},{NULL}}},
	{"date","[0-9\\-\\/\\.]*",2,1,1,{"regex",{"[0-9]{2,4}[\\-\\/]{0,1}[0-9]{1,2}[\\-\\/]{0,1}[0-9]{1,2}",NULL}},{{"date2int",{NULL}},{NULL}}},
	{NULL,NULL,-1,-1,-1,{NULL,{NULL}},{{NULL,{NULL}}}}
};

static void defineTestLibrary( strus::ContentStatisticsInterface* contentstats, strus::TextProcessorInterface* textproc)
{
	LibraryElement const* li = g_testLibrary;
	for (; li->type; ++li)
	{
		strus::TokenizerFunctionInstanceInterface* tokenizerinst;
		std::vector<strus::NormalizerFunctionInstanceInterface*> normalizers;

		const strus::TokenizerFunctionInterface* tokenizer = textproc->getTokenizer( li->tokenizer.name);
		if (!tokenizer) throw std::runtime_error( std::string("no tokenizer '") + li->tokenizer.name + "' defined");
		{
			std::vector<std::string> argar;
			char const* const* ai = li->tokenizer.arg;
			for (; *ai; ++ai)
			{
				argar.push_back( *ai);
			}
			tokenizerinst = tokenizer->createInstance( argar, textproc);
			if (!tokenizerinst) throw std::runtime_error("failed to create normalizer instance");
		}
		NormalizerDef const* ni = li->normalizers;
		for (; ni->name; ++ni)
		{
			const strus::NormalizerFunctionInterface* normalizer = textproc->getNormalizer( ni->name);
			if (!normalizer) throw std::runtime_error( std::string("no normalizer '") + ni->name + "' defined");
			std::vector<std::string> argar;
			char const* const* ai = ni->arg;
			for (; *ai; ++ai)
			{
				argar.push_back( *ai);
			}
			strus::NormalizerFunctionInstanceInterface* normalizerinst = normalizer->createInstance( argar, textproc);
			if (!normalizerinst) throw std::runtime_error("failed to create normalizer instance");
			normalizers.push_back( normalizerinst);
		}
		contentstats->addLibraryElement( li->type, li->regex, li->priority, li->minLength, li->maxLength, tokenizerinst, normalizers);
	}
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
		g_errorhnd = strus::createErrorBuffer_standard( 0, 2, NULL/*debug trace interface*/ );
		if (!g_errorhnd)
		{
			throw std::runtime_error("failed to create error buffer object");
		}
		const char* dataDir = argv[1];
		std::string mimeType = argv[2];
		const char* charset = argv[3];

		strus::local_ptr<strus::TextProcessorInterface> textproc( strus::createTextProcessor( g_errorhnd));
		if (!textproc.get()) throw std::runtime_error("unable to create text processor");
		textproc->addResourcePath( dataDir);

		strus::local_ptr<strus::DocumentClassDetectorInterface> detector( strus::createDetector_std( textproc.get(), g_errorhnd));
		if (!detector.get()) throw std::runtime_error("unable to create document class detector");
		strus::local_ptr<strus::ContentStatisticsInterface> contentstats( strus::createContentStatistics_std( textproc.get(), detector.get(), g_errorhnd));
		if (!contentstats.get()) throw std::runtime_error("unable to create content statistics");

		int ec = 0;
		defineTestLibrary( contentstats.get(), textproc.get());

		strus::local_ptr<strus::ContentStatisticsContextInterface> context( contentstats->createContext());
		if (!context.get()) throw std::runtime_error("unable to create content statistics context");

		std::vector<std::string> files;
		strus::analyzer::DocumentClass doctype( mimeType, charset);
		std::string expectfile;
		if (mimeType == "application/xml")
		{
			expectfile = "xml_expect.txt";
			ec = strus::readDirFiles( dataDir, ".xml", files);
		}
		else if (mimeType == "application/json")
		{
			expectfile = "json_expect.txt";
			ec = strus::readDirFiles( dataDir, ".json", files);
		}
		else
		{
			throw std::runtime_error(std::string("unable to process files of type MIME ") + mimeType);
		}
		if (ec) throw std::runtime_error(std::string("unable to read data directory ") + std::strerror(ec));

		std::vector<std::string>::const_iterator fi = files.begin(), fe = files.end();
		for (; fi != fe; ++fi)
		{
			std::string content;
			ec = strus::readFile( strus::joinFilePath( dataDir, *fi), content);
			if (ec) throw std::runtime_error(std::string("unable to read data file: ") + std::strerror(ec));

			std::string docid = *fi;
			context->putContent( docid, content, doctype);
		}
		if (context->nofDocuments() != (int)files.size())
		{
			throw std::runtime_error("number of documents does not match");
		}
		std::string expected;
		ec = strus::readFile( strus::joinFilePath( dataDir, expectfile), expected);
		if (ec) throw std::runtime_error(std::string("unable to read expected file: ") + std::strerror(ec));

		std::ostringstream out;
		std::vector<strus::analyzer::ContentStatisticsItem> statistics = context->statistics();
		std::vector<strus::analyzer::ContentStatisticsItem>::const_iterator si = statistics.begin(), se = statistics.end();
		for (; si != se; ++si)
		{
			out << "item select='" << si->select() << "', type='" << si->type() << "', example=[" << si->example() << "] df=" << si->df() << " tf=" << si->tf() << std::endl;
		}
		if (out.str() != expected)
		{
			std::cerr << "[OUTPUT]" << std::endl << out.str() << std::endl;
			std::cerr << "[EXPECTED]" << std::endl << expected << std::endl;
			throw std::runtime_error( "test output does not match");
		}
		const char* err = g_errorhnd->fetchError();
		if (err)
		{
			std::cerr << "has error '" << err << "'" << std::endl;
		}
		else
		{
			std::cerr << "OK" << std::endl;
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
		const char* msg = g_errorhnd ? g_errorhnd->fetchError() : "";
		std::cerr << "ERROR " << e.what() << ":" << (msg?msg:"") << std::endl;
	}
	catch (const std::exception& e)
	{
		const char* msg = g_errorhnd ? g_errorhnd->fetchError() : "";
		std::cerr << "EXCEPTION " << e.what() << ":" << (msg?msg:"") << std::endl;
	}
	if (g_errorhnd)
	{
		delete g_errorhnd;
	}
	return 1;
}
