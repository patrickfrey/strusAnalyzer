/*
 * Copyright (c) 2018 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "strus/errorBufferInterface.hpp"
#include "strus/debugTraceInterface.hpp"
#include "strus/lib/error.hpp"
#include "strus/lib/textproc.hpp"
#include "strus/lib/analyzer.hpp"
#include "strus/lib/analyzer_prgload_std.hpp"
#include "strus/lib/segmenter_textwolf.hpp"
#include "strus/lib/pattern_test.hpp"
#include "strus/lib/filelocator.hpp"
#include "strus/fileLocatorInterface.hpp"
#include "strus/textProcessorInterface.hpp"
#include "strus/documentAnalyzerContextInterface.hpp"
#include "strus/documentAnalyzerInstanceInterface.hpp"
#include "strus/segmenterInterface.hpp"
#include "private/internationalization.hpp"
#include "strus/base/fileio.hpp"
#include "strus/base/local_ptr.hpp"
#include "strus/base/string_format.hpp"
#include "strus/base/string_conv.hpp"
#include <iostream>
#include <sstream>
#include <cstring>
#include <memory>
#include <vector>
#include <string>
#include <fstream>
#include <streambuf>
#include <stdexcept>
#include <algorithm>

#undef STRUS_LOWLEVEL_DEBUG

static strus::ErrorBufferInterface* g_errorhnd = 0;
static strus::FileLocatorInterface* g_fileLocator = 0;

static void printUsage( int argc, const char* argv[])
{
	std::cerr << "usage: " << argv[0] << " [options] <resourcedir> <prgfile> <xmldoc> <expectfile>" << std::endl;
	std::cerr << "<resourcedir> = analyzer resource directory" << std::endl;
	std::cerr << "<prgfile>     = path of the file with the analyzer program to load" << std::endl;
	std::cerr << "<xmldoc>      = xml file to test document analysis on" << std::endl;
	std::cerr << "<expectfile>  = text file with the expected output" << std::endl;
	std::cerr << "options:"<< std::endl;
	std::cerr << "-h|--help         = print this usage" << std::endl;
	std::cerr << "-G|--debug <SEL>  = print debug info of name <SEL>" << std::endl;
}

static std::string getFilePath( const std::string& resourcedir, const std::string& filename)
{
	return (strus::isExplicitPath( filename)) ? filename : strus::joinFilePath( resourcedir, filename);
}

static bool compareContent( const std::string& one, const std::string& two)
{
	char const* si = one.c_str();
	char const* oi = two.c_str();
	while (*si && *oi)
	{
		bool eoln_si = false; for (;*si == '\r' || *si == '\n'; ++si,eoln_si=true){}
		bool eoln_oi = false; for (;*oi == '\r' || *oi == '\n'; ++oi,eoln_oi=true){}
		if (eoln_si != eoln_oi || *si != *oi) return false;
		if (*si && *oi) {++si;++oi;}
	}
	return *si == '\0' && *oi == '\0';
}

int main( int argc, const char* argv[])
{
	try
	{
		// Parse arguments:
		int argi = 1;
		std::vector<std::string> debugselectors;
		for (; argi < argc && argv[ argi][ 0] == '-'; ++argi)
		{
			if (std::strcmp( argv[ argi], "-h") == 0 || std::strcmp( argv[ argi], "--help") == 0)
			{
				printUsage( argc, argv);
				return 0;
			}
			else if (std::strcmp( argv[ argi], "-G") == 0 || std::strcmp( argv[ argi], "--debug") == 0)
			{
				if (++argi == argc)
				{
					printUsage( argc, argv);
					return 0;
				}
				debugselectors.push_back( argv[argi]);
			}
			else if (argv[ argi][ 1] == '-')
			{
				++argi;
				break;
			}
			else
			{
				std::cerr << "unknown option " << argv[argi] << std::endl;
				printUsage( argc, argv);
				return -1;
			}
		}
		if (argc - argi < 4)
		{
			std::cerr << "too few arguments" << std::endl;
			printUsage( argc, argv);
			return -2;
		}
		else if (argc - argi > 4)
		{
			std::cerr << "too many arguments" << std::endl;
			printUsage( argc, argv);
			return -2;
		}
		const char* resourceDir = argv[argi+0];
		std::string programFile = argv[ argi+1];
		std::string docFileName = argv[ argi+2];
		std::string expectFileName = argv[ argi+3];

		// Select configured debug traces:
		strus::DebugTraceInterface* dbgtrace = NULL;
		if (!debugselectors.empty())
		{
			dbgtrace = strus::createDebugTrace_standard( 2/*threads*/);
			if (!dbgtrace)
			{
				throw std::runtime_error("failed to create debug trace object");
			}
			std::vector<std::string>::const_iterator di = debugselectors.begin(), de = debugselectors.end();
			for (; di != de; ++di)
			{
				if (!dbgtrace->enable( *di))
				{
					throw std::runtime_error( "failed to build debug trace object");
				}
			}
		}
		g_errorhnd = strus::createErrorBuffer_standard( 0, 2/*threads*/, dbgtrace);
		if (!g_errorhnd) throw std::runtime_error("failed to create error buffer object");
		g_fileLocator = strus::createFileLocator_std( g_errorhnd);
		if (!g_fileLocator) throw std::runtime_error("failed to create file locator");

		// Create objects:
		strus::local_ptr<strus::TextProcessorInterface> textproc( strus::createTextProcessor( g_fileLocator, g_errorhnd));
		if (!textproc.get()) throw std::runtime_error( "failed to create textprocessor");

		g_fileLocator->addResourcePath( resourceDir);
		std::string docFilePath = getFilePath( resourceDir, docFileName);
		std::string expectFilePath = getFilePath( resourceDir, expectFileName);

		strus::PatternLexerInterface* patternLexer = strus::createPatternLexer_test( g_errorhnd);
		if (!patternLexer) throw std::runtime_error( "failed to create pattern lexer");
		textproc->definePatternLexer( "test", patternLexer);

		strus::PatternMatcherInterface* patternMatcher = strus::createPatternMatcher_test( g_errorhnd);
		if (!patternMatcher) throw std::runtime_error( "failed to create pattern matcher");
		textproc->definePatternMatcher( "test", patternMatcher);

		strus::local_ptr<strus::SegmenterInterface> segmenter( strus::createSegmenter_textwolf( g_errorhnd));
		if (!segmenter.get()) throw std::runtime_error( "failed to create XML segmenter");

		strus::local_ptr<strus::DocumentAnalyzerInstanceInterface> analyzer( createDocumentAnalyzer( textproc.get(), segmenter.get(), strus::analyzer::SegmenterOptions(), g_errorhnd));
		if (!analyzer.get()) throw std::runtime_error( "failed to create document analyzer");

		// Read input:
		std::string docContent;
		std::string expectContent;

		int ec;
		ec = strus::readFile( docFilePath, docContent);
		if (ec) throw std::runtime_error( strus::string_format("error reading file '%s' with document to process: %s", docFilePath.c_str(), std::strerror(ec)));
		ec = strus::readFile( expectFilePath, expectContent);
		if (ec) throw std::runtime_error( strus::string_format("error reading file '%s' with expected test output: %s", expectFilePath.c_str(), std::strerror(ec)));
		if (!strus::load_DocumentAnalyzer_programfile_std( analyzer.get(), textproc.get(), programFile, g_errorhnd))
		{
			throw std::runtime_error( strus::string_format("failed to load analyzer program in file '%s'", programFile.c_str()));
		}

		// Do the job:
		strus::analyzer::DocumentClass documentClass( "application/xml", "UTF-8");
		strus::local_ptr<strus::DocumentAnalyzerContextInterface> context( analyzer->createContext( documentClass));
		if (!context.get()) throw std::runtime_error( "failed to create document analyzer context");

		context->putInput( docContent.c_str(), docContent.size(), true/*EOF*/);
		strus::analyzer::Document doc;
		std::ostringstream out;
		while (context->analyzeNext( doc))
		{
			out << strus::string_format( "DOC %s\n", doc.subDocumentTypeName().c_str());
			{
				std::vector<strus::analyzer::DocumentAttribute>::const_iterator ai = doc.attributes().begin(), ae = doc.attributes().end();
				for (; ai != ae; ++ai)
				{
					out << strus::string_format( "AT %s %s\n", ai->name().c_str(), ai->value().c_str());
				}
			}{
				std::vector<strus::analyzer::DocumentMetaData>::const_iterator mi = doc.metadata().begin(), me = doc.metadata().end();
				for (; mi != me; ++mi)
				{
					out << strus::string_format( "MD %s %s\n", mi->name().c_str(), mi->value().tostring( 4).c_str());
				}
			}{
				std::vector<strus::analyzer::DocumentTerm> terms = doc.searchIndexTerms();
				std::sort( terms.begin(), terms.end());
				std::vector<strus::analyzer::DocumentTerm>::const_iterator si = terms.begin(), se = terms.end();
				for (; si != se; ++si)
				{
					out << strus::string_format( "SE %d %s '%s'\n", si->pos(), si->type().c_str(), si->value().c_str());
				}
			}{
				std::vector<strus::analyzer::DocumentTerm> terms = doc.forwardIndexTerms();
				std::sort( terms.begin(), terms.end());
				std::vector<strus::analyzer::DocumentTerm>::const_iterator fi = terms.begin(), fe = terms.end();
				for (; fi != fe; ++fi)
				{
					out << strus::string_format( "FW %d %s '%s'\n", fi->pos(), fi->type().c_str(), fi->value().c_str());
				}
			}
			out << "--" << std::endl << std::endl;
		}
		// Dump debug output selected (options -G <sel>):
		if (dbgtrace)
		{
			if (!dumpDebugTrace( dbgtrace, NULL/*filename*/))
			{
				std::cerr << "failed to dump debug trace" << std::endl;
			}
		}
		// Verify output:
		if (g_errorhnd->hasError()) throw std::runtime_error( "document analysis failed");
		std::string output = out.str();
		if (!compareContent( strus::string_conv::trim( expectContent), strus::string_conv::trim( output)))
		{
			std::cout << output << std::endl;
			throw std::runtime_error("test output differs");
		}
		std::cerr << "OK" << std::endl;
		return 0;
	}
	catch (const std::bad_alloc&)
	{
		std::cerr << "ERROR memory allocation error" << std::endl;
	}
	catch (const std::runtime_error& e)
	{
		const char* errmsg = g_errorhnd && g_errorhnd->hasError() ? g_errorhnd->fetchError() : 0;
		if (errmsg)
		{
			std::cerr << "ERROR " << e.what() << ":" << errmsg << std::endl;
		}
		else
		{
			std::cerr << "ERROR " << e.what() << std::endl;
		}
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
