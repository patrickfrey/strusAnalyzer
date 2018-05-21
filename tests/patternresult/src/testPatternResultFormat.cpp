/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Test of document analysis with a focus of binding terms to ordinal positions
#include "strus/lib/pattern_resultformat.hpp"
#include "strus/lib/error.hpp"
#include "strus/errorBufferInterface.hpp"
#include "strus/debugTraceInterface.hpp"
#include "strus/analyzer/patternMatcherResultItem.hpp"
#include "strus/base/local_ptr.hpp"
#include "strus/base/string_format.hpp"
#include <memory>
#include <algorithm>
#include <string>
#include <cstring>
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <set>
#include <vector>


static strus::ErrorBufferInterface* g_errorhnd = 0;

static void printUsage( int argc, const char* argv[])
{
	std::cerr << "usage: " << argv[0] << " [options]" << std::endl;
	std::cerr << "options: -h|--help      :show this usage" << std::endl;
	std::cerr << "         -V|--verbose   :verbose output" << std::endl;
}

class ThisPatternResultFormatVariableMap
	:public strus::PatternResultFormatVariableMap
{
public:
	ThisPatternResultFormatVariableMap(){}
	virtual ~ThisPatternResultFormatVariableMap(){}

	virtual const char* getVariable( const std::string& name) const
	{
		std::pair<std::set<std::string>::iterator,bool> vv = m_set.insert( name);
		return vv.first->c_str();
	}
private:
	mutable std::set<std::string> m_set;
};


struct ItemPosition
{
	int seg;
	int ofs;
};

struct ResultItemDef
{
	const char* name;
	const char* value;
	int ordstart;
	int ordend;
	ItemPosition origstart;
	ItemPosition origend;
};

struct TestDef
{
	const char* testname;
	const char* formatstring;
	ResultItemDef items[ 32];
	const char* expected;
};

static TestDef tests[ 32] =
{
	{"mapping a constant value","constant", {{NULL,NULL,0,0,{0,0},{0,0}}}, "constant"},
	{"single variable substitution 1","bla{Variable}blu", {{"Variable","(Variable:AssignedValue)"},{NULL,NULL,0,0,{0,0},{0,0}}}, "bla(Variable:AssignedValue)blu"},
	{"single variable substitution 2","bla{Variable}", {{"Variable","(Variable:AssignedValue)"},{NULL,NULL,0,0,{0,0},{0,0}}}, "bla(Variable:AssignedValue)"},
	{"double variable substitution 1","bla{Var1}{Var2}blu", {{"Var1","(Var1:AssignedValue1)"},{"Var2","(Var2:AssignedValue2)"},{NULL,NULL,0,0,{0,0},{0,0}}}, "bla(Var1:AssignedValue1)(Var2:AssignedValue2)blu"},
	{"double variable substitution 2","bla{Var2}{Var1}blu", {{"Var1","(Var1:AssignedValue1)"},{"Var2","(Var2:AssignedValue2)"},{NULL,NULL,0,0,{0,0},{0,0}}}, "bla(Var2:AssignedValue2)(Var1:AssignedValue1)blu"},
	{"array variable substitution 1","bla{Variable|,}blu", {{"Variable","(Var1:AssignedValue1)"},{"Variable","(Var2:AssignedValue2)"},{NULL,NULL,0,0,{0,0},{0,0}}}, "bla(Var1:AssignedValue1),(Var2:AssignedValue2)blu"},
	{"array variable substitution 2","{Variable|,}blu", {{"Variable","(Var1:AssignedValue1)"},{"Variable","(Var2:AssignedValue2)"},{"Variable","(Var3:AssignedValue3)"},{NULL,NULL,0,0,{0,0},{0,0}}}, "(Var1:AssignedValue1),(Var2:AssignedValue2),(Var3:AssignedValue3)blu"},
	{"escaping curly brackets 1","bla\\{{Variable}blu", {{"Variable","(Variable:AssignedValue)"},{NULL,NULL,0,0,{0,0},{0,0}}}, "bla{(Variable:AssignedValue)blu"},
	{"escaping curly brackets 2","bla\\}{Variable}blu", {{"Variable","(Variable:AssignedValue)"},{NULL,NULL,0,0,{0,0},{0,0}}}, "bla}(Variable:AssignedValue)blu"},
	{"escaping curly brackets 3","bla\\{\\{\\{{Variable}blu", {{"Variable","(Variable:AssignedValue)"},{NULL,NULL,0,0,{0,0},{0,0}}}, "bla{{{(Variable:AssignedValue)blu"},
	{"escaping curly brackets 4","bla{Variable}\\{blu", {{"Variable","(Variable:AssignedValue)"},{NULL,NULL,0,0,{0,0},{0,0}}}, "bla(Variable:AssignedValue){blu"},
	{NULL,NULL,{},NULL}
};

int main( int argc, const char* argv[])
{
	int argi = 1;
	bool verbose = false;
	for (; argi < argc && argv[argi][0] == '-'; ++argi)
	{
		if (std::strcmp( argv[argi], "-h") == 0 || std::strcmp( argv[argi], "--help") == 0)
		{
			printUsage( argc, argv);
			return 0;
		}
		else if (std::strcmp( argv[argi], "-V") == 0 || std::strcmp( argv[argi], "--verbose") == 0)
		{
			verbose = true;
		}
		else if (std::strcmp( argv[argi], "--") == 0)
		{
			argi++;
			break;
		}
		else
		{
			std::cerr << "ERROR unknown option " << argv[argi] << std::endl;
			printUsage( argc, argv);
			return 1;
		}
	}
	if (argc-argi > 0)
	{
		std::cerr << "ERROR too many parameters" << std::endl;
		printUsage( argc, argv);
		return 1;
	}
	try
	{
		strus::DebugTraceInterface* debugtrace = NULL;
		if (verbose)
		{
			debugtrace = strus::createDebugTrace_standard( 2);
			if (!debugtrace)
			{
				std::cerr << "ERROR failed to create debug trace interface" << std::endl;
			}
			debugtrace->enable( "pattern");
		}
		g_errorhnd = strus::createErrorBuffer_standard( 0, 2, debugtrace);
		if (!g_errorhnd)
		{
			throw std::runtime_error("failed to create error buffer object");
		}
		ThisPatternResultFormatVariableMap varmap;
		strus::PatternResultFormatTable formatTable( g_errorhnd, &varmap);
		strus::PatternResultFormatContext context( g_errorhnd);

		TestDef const* ti = tests;
		for (int tidx=1; ti->formatstring; ++ti,++tidx)
		{
			if (verbose)
			{
				std::cerr << "Executing test [" << tidx << "] " << ti->testname << std::endl;
			}
			const strus::PatternResultFormat* fmt = formatTable.createResultFormat( ti->formatstring);
			if (!fmt)
			{
				throw std::runtime_error( g_errorhnd->fetchError());
			}
			std::vector<strus::analyzer::PatternMatcherResultItem> items;
			ResultItemDef const* ii = ti->items;
			for (; ii->name; ++ii)
			{
				const char* varname = varmap.getVariable( ii->name);
				if (!varname)
				{
					throw std::runtime_error( std::string("undefined variable '") + ii->name + "'");
				}
				items.push_back( strus::analyzer::PatternMatcherResultItem(
					varname, ii->value, ii->ordstart, ii->ordend,
					ii->origstart.seg, ii->origstart.ofs,
					ii->origend.seg, ii->origend.ofs));
			}
			const char* res = context.map( fmt, items.size(), items.data());
			if (!res)
			{
				throw std::runtime_error( g_errorhnd->fetchError());
			}
			std::ostringstream fmtout;
			strus::PatternResultFormatChunk chunk;
			char const* ri = res;
			while (strus::PatternResultFormatChunk::parseNext( chunk, ri))
			{
				if (chunk.value)
				{
					fmtout << std::string( chunk.value, chunk.valuesize);
				}
				else
				{
					fmtout << strus::string_format(
							"[start=(%d,%d) end=(%d,%d)]",
							chunk.start_seg, chunk.start_pos,
							chunk.end_seg, chunk.end_pos);
				}
			}
			std::string out = fmtout.str();
			if (out != ti->expected)
			{
				std::cout << "[OUTPUT]" << std::endl << out << std::endl;
				std::cout << "[EXPECT]" << std::endl << ti->expected << std::endl;
				throw std::runtime_error( "test output not as expected");
			}
		}
		if (g_errorhnd->hasError())
		{
			throw std::runtime_error( g_errorhnd->fetchError());
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


