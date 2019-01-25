/*
 * Copyright (c) 2017 Patrick Frey <patrickpfrey@yahoo.com>
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
#include <iostream>
#include <cstring>
#include <memory>
#include <vector>
#include <string>
#include <fstream>
#include <streambuf>
#include <stdexcept>

static strus::ErrorBufferInterface* g_errorhnd = 0;
static strus::FileLocatorInterface* g_fileLocator = 0;

struct Token
{
	const char* value;
	int pos;
	int len;
};

struct Test
{
	const char* regex;
	int index;
	const char* input;
	const Token output[20];
};

static const Test g_test[] =
{
	{"[Aa]", 0, "Abcdes", {{"A",0,1},{0,0,0}}},
	{"[Aa]+", 0, "aAbcdes", {{"aA",0,2},{0,0,0}}},
	{"[Aa]+", 0, "xAbcdes", {{"A",1,1},{0,0,0}}},
	{"[Aa]+", 0, "xAaabcdes", {{"Aaa",1,3},{0,0,0}}},
	{"b([Aa]+)", 1, "xbAaabcdes", {{"Aaa",2,3},{0,0,0}}},
	{"b([Aa]+)b", 1, "xbAaabcdes", {{"Aaa",2,3},{0,0,0}}},
	{"([Aa]+)b", 1, "xbAaabcdes", {{"Aaa",2,3},{0,0,0}}},
	{"^[a-z]{3,6}[:][/][/]([^ ?]*)", 1, "https://bla.world.com?g=3&k=1", {{"bla.world.com", 8, 13},{0,0,0}}},
	{"^([a-z]{1,32}[.][^ ?]*)", 1, "bla.world.com?g=3&k=1", {{"bla.world.com", 0, 13},{0,0,0}}},
	{"^[a-z]{1,32}[.]([^ ?]*)", 1, "https://bla.world.com?g=3&k=1", {{0,0,0}}},
	{0,0,0,{{0,0,0}}}
};

static void printTokens( std::ostream& out, const std::vector<strus::analyzer::Token>& tokens, const char* src)
{
	std::vector<strus::analyzer::Token>::const_iterator ri = tokens.begin(), re = tokens.end();
	for (int ridx=0; ri != re; ++ri,++ridx)
	{
		std::string rval( src + ri->origpos().ofs(), ri->origsize());
		if (ridx) out << ", ";
		out << "(" << ri->origpos().ofs() << "," << ri->origsize() << ",'" << rval << "')";
	}
}

int main( int argc, const char* argv[])
{
	int rt = 0;
	try
	{
		g_errorhnd = strus::createErrorBuffer_standard( 0, 2/*threads*/, NULL);
		if (!g_errorhnd) throw std::runtime_error("failed to create error buffer object");
		g_fileLocator = strus::createFileLocator_std( g_errorhnd);
		if (!g_fileLocator) throw std::runtime_error("failed to create file locator");
		strus::local_ptr<strus::TextProcessorInterface> textproc( strus::createTextProcessor( g_fileLocator, g_errorhnd));
		if (!textproc.get()) throw std::runtime_error("failed to create text processor");

		const strus::TokenizerFunctionInterface* tokenizer = textproc->getTokenizer( "regex");
		if (!tokenizer)
		{
			throw std::runtime_error( "tokenizer 'regex' not defined");
		}

		Test const* ti = g_test;
		for (int tidx=1; ti->regex; ++ti,++tidx)
		{
			std::cerr << "[" << tidx << "] matching " << ti->regex << " index " << ti->index << " on " << ti->input;
			std::vector<std::string> args;
			char indexstr[ 32];
			std::snprintf( indexstr, sizeof(indexstr), "%d", ti->index);
			args.push_back( ti->regex);
			args.push_back( indexstr);
			strus::local_ptr<strus::TokenizerFunctionInstanceInterface> inst( tokenizer->createInstance( args, textproc.get()));
			if (!inst.get()) throw std::runtime_error( "failed to create tokenizer");

			std::vector<strus::analyzer::Token> tokens( inst->tokenize( ti->input, std::strlen(ti->input)));
			std::vector<strus::analyzer::Token>::const_iterator ri = tokens.begin(), re = tokens.end();
			Token const* ei = ti->output;
			bool success = true;
			for (; ri != re && ei->value; ++ri,++ei)
			{
				if (ei->pos != ri->origpos().ofs())
				{
					std::cerr << "match position not as expected: " << ei->pos << " != " << ri->origpos().ofs() << std::endl;
					success = false;
				}
				if (ei->len != ri->origsize())
				{
					std::cerr << "match length not as expected: " << ei->len << " != " << ri->origsize() << std::endl;
					success = false;
				}
				std::string rval( ti->input + ri->origpos().ofs(), ei->len);
				if (rval != ei->value)
				{
					std::cerr << "match value not as expected: '" << rval << "' != '" << ei->value << "'" << std::endl;
					success = false;
				}
			}
			std::cerr << " results: ";
			printTokens( std::cerr, tokens, ti->input);
			std::cerr << std::endl;

			if (ri != re || ei->value || !success)
			{
				std::cerr << std::endl;
				if (ri != re) std::cerr << "more results than expected" << std::endl;
				if (ei->value) std::cerr << "less results than expected" << std::endl;
				throw std::runtime_error( "results not as expected");
			}
		}
		std::cerr << "done." << std::endl;
		rt = 0;
	}
	catch (const std::bad_alloc&)
	{
		std::cerr << std::endl << "ERROR memory allocation error" << std::endl;
		rt = 2;
	}
	catch (const std::runtime_error& e)
	{
		if (g_errorhnd && g_errorhnd->hasError())
		{
			std::cerr << std::endl << "ERROR " << e.what() << ":" << g_errorhnd->fetchError() << std::endl;
		}
		else
		{
			std::cerr << std::endl << "ERROR " << e.what() << std::endl;
		}
		rt = 1;
	}
	catch (const std::exception& e)
	{
		std::cerr << std::endl << "EXCEPTION " << e.what() << std::endl;
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

