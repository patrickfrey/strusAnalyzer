/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Test of the document markup of the textwolf based XML segmenter
#include "strus/lib/markup_document_tags.hpp"
#include "strus/lib/error.hpp"
#include "strus/lib/filelocator.hpp"
#include "strus/lib/textproc.hpp"
#include "strus/fileLocatorInterface.hpp"
#include "strus/errorBufferInterface.hpp"
#include "strus/textProcessorInterface.hpp"
#include "strus/analyzer/documentClass.hpp"
#include "strus/analyzer/documentAttribute.hpp"
#include "strus/base/local_ptr.hpp"
#include "strus/base/fileio.hpp"
#include "strus/base/string_format.hpp"
#include "tree.hpp"
#include <memory>
#include <string>
#include <vector>
#include <cstring>
#include <stdexcept>
#include <iostream>
#include <sstream>

static strus::ErrorBufferInterface* g_errorhnd = 0;
static strus::FileLocatorInterface* g_fileLocator = 0;
static bool g_verbose = false;

static void printUsage( int argc, const char* argv[])
{
	std::cerr << "usage: " << argv[0] << " [-v,-V] [<noftests> <complexity>]" << std::endl;
	std::cerr << "-h = print this usage" << std::endl;
	std::cerr << "-V = verbose output" << std::endl;
	std::cerr << "<noftests>   : number of tests (default 100)" << std::endl;
	std::cerr << "<complexity> : complexity of tests (default 100)" << std::endl;
}

class TestDocumentItem
{
public:
	enum Type {Content,Tag};

	TestDocumentItem( Type type_, const std::string& value_, const std::vector<strus::analyzer::DocumentAttribute>& attributes_ = std::vector<strus::analyzer::DocumentAttribute>())
		:m_type(type_),m_value(value_),m_attributes(attributes_){}
	TestDocumentItem( const TestDocumentItem& o)
		:m_type(o.m_type),m_value(o.m_value),m_attributes(o.m_attributes){}

	Type type() const							{return m_type;}
	const std::string& value() const					{return m_value;}
	const std::vector<strus::analyzer::DocumentAttribute>& attributes() const		{return m_attributes;}

private:
	Type m_type;
	std::string m_value;
	std::vector<strus::analyzer::DocumentAttribute> m_attributes;
};

typedef strus::test::TreeNode<TestDocumentItem> TestDocumentTree;


int main( int argc, const char* argv[])
{
	int rt = 0;
	int ec = 0;
	int argi = 1;
	int nofTests = 100;
	int complexity = 100;
	for (; argi < argc; ++argi)
	{
		if (std::strcmp( argv[argi], "-h") == 0 || std::strcmp( argv[argi], "--help") == 0)
		{
			printUsage( argc, argv);
			return 0;
		}
		else if (std::strcmp( argv[argi], "-V") == 0 || std::strcmp( argv[argi], "--verbose") == 0)
		{
			g_verbose = true;
		}
	}
	if (argi < argc)
	{
		nofTests = atoi( argv[argi]);
		if (nofTests <= 0)
		{
			std::cerr << "bad argument for number of tests: " << argv[argi] << std::endl;
			exit( 1);
		}
	}
	if (argi < argc)
	{
		complexity = atoi( argv[argi]);
		if (complexity <= 0)
		{
			std::cerr << "bad argument for complexity of tests: " << argv[argi] << std::endl;
			return 1;
		}
	}
	if (argi < argc)
	{
		std::cerr << "too many arguments (maximum 2)" << std::endl;
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

		if (g_errorhnd->hasError())
		{
			throw std::runtime_error( g_errorhnd->fetchError());
		}
		std::string result;
		std::string expected;

		if (result != expected)
		{
			ec = strus::writeFile( "RES", result);
			if (ec) throw std::runtime_error( strus::string_format( "error writing file %s: %s", "RES", ::strerror(ec)));
			ec = strus::writeFile( "EXP", expected);
			if (ec) throw std::runtime_error( strus::string_format( "error writing file %s: %s", "EXP", ::strerror(ec)));
			throw std::runtime_error("output not as expected");
		}
		std::cerr << "OK" << std::endl;
		rt = 0;
	}
	catch (const std::bad_alloc&)
	{
		std::ostringstream msg;
		if (g_errorhnd->hasError())
		{
			msg << "(" << g_errorhnd->fetchError() << ")";
		}
		std::cerr << "ERROR memory allocation error" << msg.str() << std::endl;
		rt = 2;
	}
	catch (const std::runtime_error& e)
	{
		std::ostringstream msg;
		if (g_errorhnd->hasError())
		{
			msg << "(" << g_errorhnd->fetchError() << ")";
		}
		std::cerr << "ERROR " << e.what() << msg.str() << std::endl;
		rt = 1;
	}
	catch (const std::exception& e)
	{
		std::ostringstream msg;
		if (g_errorhnd->hasError())
		{
			msg << "(" << g_errorhnd->fetchError() << ")";
		}
		std::cerr << "EXCEPTION " << e.what() << msg.str() << std::endl;
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


