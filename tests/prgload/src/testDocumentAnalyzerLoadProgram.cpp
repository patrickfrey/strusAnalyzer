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
#include "strus/lib/analyzer.hpp"
#include "strus/lib/analyzer_prgload_std.hpp"
#include "strus/lib/segmenter_textwolf.hpp"
#include "strus/textProcessorInterface.hpp"
#include "strus/documentAnalyzerContextInterface.hpp"
#include "strus/documentAnalyzerInterface.hpp"
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

static void printUsage( int argc, const char* argv[])
{
	std::cerr << "usage: " << argv[0] << " <resourcedir> <prgfile> <xmldoc> <expectfile>" << std::endl;
	std::cerr << "<resourcedir> = analyzer resource directory" << std::endl;
	std::cerr << "<prgfile>     = path of the file with the analyzer program to load" << std::endl;
	std::cerr << "<xmldoc>      = xml file to test document analysis on" << std::endl;
	std::cerr << "<expectfile>  = text file with the expected output" << std::endl;
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
		std::cerr << "ERROR too few arguments" << std::endl;
		printUsage( argc, argv);
		return 1;
	}
	else if (argc > 5)
	{
		std::cerr << "ERROR too many arguments" << std::endl;
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
		const char* workingDir = argv[1];
		std::string programFile = strus::joinFilePath( workingDir, argv[2]);
		std::string docFile = strus::joinFilePath( workingDir, argv[3]);
		std::string expectFile = strus::joinFilePath( workingDir, argv[4]);

		strus::local_ptr<strus::TextProcessorInterface> textproc( strus::createTextProcessor( g_errorhnd));
		if (!textproc.get()) throw std::runtime_error( "failed to create textprocessor");
		textproc->addResourcePath( workingDir);
		strus::local_ptr<strus::SegmenterInterface> segmenter( strus::createSegmenter_textwolf( g_errorhnd));
		if (!segmenter.get()) throw std::runtime_error( "failed to create XML segmenter");

		strus::local_ptr<strus::DocumentAnalyzerInterface> analyzer( createDocumentAnalyzer( textproc.get(), segmenter.get(), strus::analyzer::SegmenterOptions(), g_errorhnd));
		if (!analyzer.get()) throw std::runtime_error( "failed to create document analyzer");

		std::string programContent;
		std::string docContent;
		std::string expectContent;
		std::vector<std::string> warnings;

		int ec;
		ec = strus::readFile( programFile, programContent);
		if (ec) throw std::runtime_error( strus::string_format("error reading analyzer program file '%s': %s", programFile.c_str(), std::strerror(ec)));
		ec = strus::readFile( docFile, docContent);
		if (ec) throw std::runtime_error( strus::string_format("error reading file '%s' with document to process: %s", docFile.c_str(), std::strerror(ec)));
		ec = strus::readFile( expectFile, expectContent);
		if (ec) throw std::runtime_error( strus::string_format("error reading file '%s' with expected test output: %s", expectFile.c_str(), std::strerror(ec)));
		if (!strus::load_DocumentAnalyzer_program_std( analyzer.get(), textproc.get(), programContent, warnings, g_errorhnd))
		{
			throw std::runtime_error( strus::string_format("failed to load analyzer program in file '%s'", programFile.c_str()));
		}
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
		if (g_errorhnd->hasError()) throw std::runtime_error( "document analysis failed");
		std::string output = out.str();
		if (strus::string_conv::trim( expectContent) != strus::string_conv::trim( output))
		{
			std::cout << output << std::endl;
			throw std::runtime_error("test outpu differs");
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
