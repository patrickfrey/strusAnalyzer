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

#undef STRUS_LOWLEVEL_DEBUG

static strus::ErrorBufferInterface* g_errorhnd = 0;
static strus::FileLocatorInterface* g_fileLocator = 0;

struct Test
{
	const char* regex;
	const char* format;
	const char* input;
	const char* output;
};

static const Test g_test[] =
{
	{"[Aa]", "$0", "A", "A"},
	{"Aa", "$0", "Aa", "Aa"},
	{"[Aa]a", "$0", "Aa", "Aa"},
	{"[Aa]+", "$0", "Aa", "Aa"},
	{"[a-c]", "$0", "b", "b"},
	{"[a-c]+", "$0", "ba", "ba"},
	{"[Aa]+([0123456789]+)[az]*", "$1", "A17az", "17"},
	{"[abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ]+([0123456789]+)[abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ ]*", "$1", "Abracadabra17 is a bad password", "17"},
	{"[a-zA-Z]+([0-9]+)", "$1", "Abracadabra17", "17"},
	{"^[^0-9]+", NULL, "Abracadabra17", "Abracadabra"},
	{"[^0-9]+", NULL, "Abrac4dabra17", "\0Abrac\0dabra\0\0"},
	{"[a-z]+", NULL, "Abrac4dabra17a", "\0brac\0dabra\0a\0\0"},
	{"[a-zA-Z]+([0-9]+)a", "$1", "Abracadabra17a", "17"},
	{"[a-zA-Z]+([0-9]+)[a-z ]+", "$1", "Abracadabra17 is a bad password", "17"},
	{"[a-zA-Z]+([0-9]+)[a-z ]+([0-9]+)[a-z ]+", "$1", "Abracadabra17 or 18 is a bad password", "17"},
	{0,0,0,0}
};

std::string mapOutputPrintable( const char* output, int outputsize=-1)
{
	if (!output)
	{
		return "NULL";
	}
	else if (output[0] == '\0')
	{
		std::string rt;
		rt.push_back( '{');
		char const* oi = output;
		char const* oe;
		if (outputsize < 0)
		{
			for (oe=oi; oe[0] || oe[1]; ++oe){}
		}
		else
		{
			oe = output + outputsize;
		}
		int oidx = 0;
		for (++oi; oi < oe; ++oi,++oidx)
		{
			if (oidx) rt.append(", ");
			char const* pi = oi;
			oi = std::strchr( oi, '\0');
			rt.append( pi, oi-pi);
		}
		rt.push_back( '}');
		return rt;
	}
	else
	{
		return strus::string_format( "'%s'", output);
	}
}

std::string mapOutput( const char* output)
{
	std::string rt;
	if (!output)
	{}
	else if (output[0] == '\0')
	{
		char const* oi = output;
		for (; oi[0] || oi[1]; ++oi){}
		rt.append( output, oi - output);
	}
	else
	{
		rt.append( output);
	}
	return rt;
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

		const strus::NormalizerFunctionInterface* normalizer = textproc->getNormalizer( "regex");
		if (!normalizer)
		{
			throw std::runtime_error( "normalizer 'regex' not defined");
		}

		Test const* ti = g_test;
		for (int tidx=1; ti->regex; ++ti,++tidx)
		{
			if (ti->format)
			{
				std::cerr << "[" << tidx << "] matching " << ti->regex << " write " << ti->format << " on " << ti->input;
			}
			else
			{
				std::cerr << "[" << tidx << "] matching " << ti->regex << " on " << ti->input;
			}
			std::vector<std::string> args;
			args.push_back( ti->regex);
			if (ti->format)
			{
				args.push_back( ti->format);
			}
			strus::local_ptr<strus::NormalizerFunctionInstanceInterface> inst( normalizer->createInstance( args, textproc.get()));
			if (!inst.get()) throw std::runtime_error( "failed to create normalizer");

			std::string result( inst->normalize( ti->input, std::strlen(ti->input)));
			if (result.empty())
			{
				std::cerr << std::endl;
				throw std::runtime_error( "failed to normalize");
			}
			std::string outputstr = mapOutput( ti->output);
			if (result != outputstr)
			{
				std::cerr << " got " << mapOutputPrintable( result.c_str(), result.size())
					  << " but expected " << mapOutputPrintable( ti->output)
					  << std::endl;
				throw std::runtime_error( "result not as expected");
			}
			std::cerr << " result " << mapOutputPrintable( ti->output) << std::endl;
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

