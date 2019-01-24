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
#include "strus/normalizerFunctionInstanceInterface.hpp"
#include "strus/normalizerFunctionInterface.hpp"
#include "strus/base/local_ptr.hpp"
#include "strus/base/string_format.hpp"
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

struct Test
{
	const char* normalizer;
	const char* input;
	const char* substr[ 16];
	const char* output;
};

static const Test g_test[] =
{
	{"substrindex","2011-November-17", {"","January","February","March","April","May","June","July","August","September","October","November","December",0}, "2011-11-17"},
	{"substrindex","2011-Novemberr-17", {"","January","February","March","April","May","June","July","August","September","October","November","Novemberr","December",0}, "2011-12-17"},
	{"substrindex","2011-Novemberr-17", {"","January","February","March","April","May","June","July","August","September","October","Novemberr","November","December",0}, "2011-11-17"},
	{"substrindex","2013-Apr-1", {"","Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec",0}, "2013-4-1"},
	{"substrindex","1999-C-1", {"A","B","C","D","E","F","G","H","I","J","K","L","M",0}, "1999-2-1"},
	{"substrindex","1997-F-114234412", {"A","B","C","D","E","F","G","H","I","J","K","L","M",0}, "1997-5-114234412"},
	{"substrindex","1997-F-11423A4412", {"A","B","C","D","E","F","G","H","I","J","K","L","M",0}, "1997-5-1142304412"},
	{"substrindex","ABCDEFGHI", {"","A","B","C","D","E","F","G","H","I","J","K","L","M",0}, "123456789"},
	{"substrindex","ABCDEF5GHI", {"","A","B","C","D","E","F","G","H","I","J","K","L","M",0}, "1234565789"},
	{"substrmap","ABCDEF5GHI", {"A=B,B=C,C=D,D=E,E=F,F=G", 0}, "BCDEFG5GHI"},
	{0,0,{0},0}
};

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

		Test const* ti = g_test;
		for (int tidx=1; ti->normalizer; ++ti,++tidx)
		{
			const strus::NormalizerFunctionInterface* normalizer = textproc->getNormalizer( ti->normalizer);
			if (!normalizer)
			{
				throw std::runtime_error( strus::string_format("normalizer '%s' not defined", ti->normalizer));
			}
			std::cerr << "[" << tidx << "] " << ti->normalizer << " matching " << ti->input << std::endl;
			std::vector<std::string> args;
			char const* const* si = ti->substr;
			for (; *si; ++si)
			{
				args.push_back( *si);
			}
			strus::local_ptr<strus::NormalizerFunctionInstanceInterface> inst( normalizer->createInstance( args, textproc.get()));
			if (!inst.get()) throw std::runtime_error( "failed to create normalizer");

			int inputlen = std::strlen(ti->input);
			std::string result( inst->normalize( ti->input, inputlen));
			if (result.empty())
			{
				std::cerr << std::endl;
				throw std::runtime_error( "failed to normalize");
			}
			if (result != ti->output)
			{
				std::cerr << " got '" << result << "' but expected '" << ti->output << "'" << std::endl;
				throw std::runtime_error( "result not as expected");
			}
			std::cerr << " result '" << ti->output << "'" << std::endl;
		}
		std::cerr << "OK" << std::endl;
		rt = 0;
	}
	catch (const std::bad_alloc&)
	{
		std::cerr << "ERROR memory allocation error" << std::endl;
		rt = 2;
	}
	catch (const std::runtime_error& e)
	{
		if (g_errorhnd && g_errorhnd->hasError())
		{
			std::cerr << "ERROR " << e.what() << ":" << g_errorhnd->fetchError() << std::endl;
		}
		else
		{
			std::cerr << "ERROR " << e.what() << std::endl;
		}
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

