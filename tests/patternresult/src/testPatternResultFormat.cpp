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
#include "strus/base/utf8.hpp"
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
static bool g_verbose = false;

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

struct ItemTestDef
{
	const char* testname;
	const char* formatstring;
	ResultItemDef items[ 32];
	const char* expected;
};

#define END_ResultItem {NULL,NULL,0,0,{0,0},{0,0}}

static ItemTestDef g_itemTests[ 32] =
{
	{"mapping a constant value","constant", {END_ResultItem}, "constant"},
	{"single variable substitution 1","bla{Variable}blu", {{"Variable","(Variable:AssignedValue)"},END_ResultItem}, "bla(Variable:AssignedValue)blu"},
	{"single variable substitution 2","bla{Variable}", {{"Variable","(Variable:AssignedValue)"},END_ResultItem}, "bla(Variable:AssignedValue)"},
	{"single variable substitution content 1","bla{Variable}blu", {{"Variable",NULL,101,102,{11,1},{11,7}},END_ResultItem}, "bla[start=(11,1) end=(11,7)]blu"},
	{"double variable substitution 1","bla{Var1}{Var2}blu", {{"Var1","(Var1:AssignedValue1)"},{"Var2","(Var2:AssignedValue2)"},END_ResultItem}, "bla(Var1:AssignedValue1)(Var2:AssignedValue2)blu"},
	{"double variable substitution 2","bla{Var2}{Var1}blu", {{"Var1","(Var1:AssignedValue1)"},{"Var2","(Var2:AssignedValue2)"},END_ResultItem}, "bla(Var2:AssignedValue2)(Var1:AssignedValue1)blu"},
	{"double variable substitution content 1","bla{Var1}{Var2}blu", {{"Var1",NULL,17,23,{113,18},{114,21}},{"Var2",NULL,71,75,{1,18},{1,53}},END_ResultItem}, "bla[start=(113,18) end=(114,21)][start=(1,18) end=(1,53)]blu"},
	{"array variable substitution 1","bla{Variable|,}blu", {{"Variable","(Var1:AssignedValue1)"},{"Variable","(Var2:AssignedValue2)"},END_ResultItem}, "bla(Var1:AssignedValue1),(Var2:AssignedValue2)blu"},
	{"array variable substitution 2","{Variable|,}blu", {{"Variable","(Var1:AssignedValue1)"},{"Variable","(Var2:AssignedValue2)"},{"Variable","(Var3:AssignedValue3)"},END_ResultItem}, "(Var1:AssignedValue1),(Var2:AssignedValue2),(Var3:AssignedValue3)blu"},
	{"array variable substitution content 1","bla{Variable|; }blu", {{"Variable",NULL,13,45,{1,13},{1,23}},{"Variable",NULL,112,123,{7098,113},{7109,1023}},END_ResultItem}, "bla[start=(1,13) end=(1,23)]; [start=(7098,113) end=(7109,1023)]blu"},
	{"array variable substitution content 2","bla{Variable|; }blu", {{"Variable",NULL,13,45,{1,13},{1,23}},{"Variable",NULL,112,123,{7098,113},{7109,1023}},{"Variable",NULL,1,1,{71,1},{71,1}},END_ResultItem}, "bla[start=(1,13) end=(1,23)]; [start=(7098,113) end=(7109,1023)]; [start=(71,1) end=(71,1)]blu"},
	{"escaping curly brackets 1","bla\\{{Variable}blu", {{"Variable","(Variable:AssignedValue)"},END_ResultItem}, "bla{(Variable:AssignedValue)blu"},
	{"escaping curly brackets 2","bla\\}{Variable}blu", {{"Variable","(Variable:AssignedValue)"},END_ResultItem}, "bla}(Variable:AssignedValue)blu"},
	{"escaping curly brackets 3","bla\\{\\{\\{{Variable}blu", {{"Variable","(Variable:AssignedValue)"},END_ResultItem}, "bla{{{(Variable:AssignedValue)blu"},
	{"escaping curly brackets 4","bla{Variable}\\{blu", {{"Variable","(Variable:AssignedValue)"},END_ResultItem}, "bla(Variable:AssignedValue){blu"},
	{NULL,NULL,{},NULL}
};

static void printEncodedNum( std::ostream& out, char const*& ri)
{
	if (!*ri) throw std::runtime_error("corrupt data");
	int chlen = strus::utf8charlen( *ri);
	int val = strus::utf8decode( ri, chlen);
	out << val;
	ri += chlen;
}

static std::string escapedPatternFormatOutput( const char* res)
{
	std::ostringstream out;
	char const* ri = res;
	while (*ri)
	{
		if (*ri == '\1')
		{
			out << "<1";
			++ri;
			out << "|"; printEncodedNum( out, ri);
			out << "|"; printEncodedNum( out, ri);
			out << "|"; printEncodedNum( out, ri);
			out << ">";
		}
		else if (*ri == '\2')
		{
			out << "<2";
			++ri;
			out << "|"; printEncodedNum( out, ri);
			out << "|"; printEncodedNum( out, ri);
			out << "|"; printEncodedNum( out, ri);
			out << "|"; printEncodedNum( out, ri);
			out << ">";
		}
		else
		{
			out << *ri;
			++ri;
		}
	}
	return out.str();
}

static std::vector<strus::analyzer::PatternMatcherResultItem>
	getResultItemsFromDef(
		const strus::PatternResultFormatVariableMap& varmap,
		const ResultItemDef* items)
{
	std::vector<strus::analyzer::PatternMatcherResultItem> rt;
	ResultItemDef const* ii = items;
	for (; ii->name; ++ii)
	{
		const char* varname = varmap.getVariable( ii->name);
		if (!varname)
		{
			throw std::runtime_error( std::string("undefined variable '") + ii->name + "'");
		}
		rt.push_back( strus::analyzer::PatternMatcherResultItem(
			varname, ii->value, ii->ordstart, ii->ordend,
			ii->origstart.seg, ii->origstart.ofs,
			ii->origend.seg, ii->origend.ofs));
	}
	return rt;
}

static void checkPatternMatchResult( const char* res, const char* expected)
{
	if (!res)
	{
		throw std::runtime_error( g_errorhnd->fetchError());
	}
	if (g_verbose)
	{
		std::cerr << "[MAP]" << std::endl << escapedPatternFormatOutput(res) << std::endl;
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
	if (out != expected)
	{
		std::cout << "[OUTPUT]" << std::endl << out << std::endl;
		std::cout << "[EXPECT]" << std::endl << expected << std::endl;
		throw std::runtime_error( "test output not as expected");
	}
}

static void runItemTests()
{
	ThisPatternResultFormatVariableMap varmap;
	strus::PatternResultFormatTable formatTable( &varmap, g_errorhnd);
	strus::PatternResultFormatContext context( g_errorhnd);

	std::cerr << "Executing item tests:" << std::endl;
	ItemTestDef const* ti = g_itemTests;
	for (int tidx=1; ti->testname; ++ti,++tidx)
	{
		if (g_verbose)
		{
			std::cerr << "Executing test [" << tidx << "] " << ti->testname << std::endl;
		}
		const strus::PatternResultFormat* fmt = formatTable.createResultFormat( ti->formatstring);
		if (!fmt)
		{
			throw std::runtime_error( g_errorhnd->fetchError());
		}
		std::vector<strus::analyzer::PatternMatcherResultItem> items = getResultItemsFromDef( varmap, ti->items);
		const char* res = context.map( fmt, items.data(), items.size());

		checkPatternMatchResult( res, ti->expected);
	}
}

struct ResultDef
{
	const char* name;
	int ordstart;
	int ordend;
	ItemPosition origstart;
	ItemPosition origend;
	ResultItemDef items[ 32];
};

struct ResultTestDef
{
	const char* testname;
	const char* formatstring;
	ResultDef result;
	const char* expected;
};

static ResultTestDef g_resultTests[ 32] =
{
	{"mapping a constant value","constant", {"result",13,14,{31,4},{32,7},{END_ResultItem}}, "constant"},
	{"mapping all result attributes","<ordpos>{ordpos}</ordpos><ordlen>{ordlen}</ordlen><ordend>{ordend}</ordend><startseg>{startseg}</startseg><startpos>{startpos}</startpos><endseg>{endseg}</endseg><endpos>{endpos}</endpos><name>{name}</name><value>{value}</value>", {"result",13,14,{31,4},{32,7},{END_ResultItem}}, "<ordpos>13</ordpos><ordlen>1</ordlen><ordend>14</ordend><startseg>31</startseg><startpos>4</startpos><endseg>32</endseg><endpos>7</endpos><name>result</name><value></value>"},
	{"mapping result with items","{ordpos}: {name}={value}|#{ordpos}:{name}->{value}|, ", {"result",13,14,{31,4},{32,7},{{"first","(value:first)"},{"second",NULL,41,43,{61,71},{61,79}},END_ResultItem}}, "13: result=#0:first->(value:first), #41:second->[start=(61,71) end=(61,79)]"},
	{NULL,NULL,{},NULL}
};

static strus::analyzer::PatternMatcherResult
	getResultFromDef(
		const strus::PatternResultFormatVariableMap& varmap,
		const ResultDef& result)
{
	std::vector<strus::analyzer::PatternMatcherResultItem> items = getResultItemsFromDef( varmap, result.items);
	return strus::analyzer::PatternMatcherResult( result.name, 0/*value*/, result.ordstart, result.ordend, result.origstart.seg, result.origstart.ofs, result.origend.seg, result.origend.ofs, items);
}

static void runResultTests()
{
	std::cerr << "Executing result tests:" << std::endl;
	ThisPatternResultFormatVariableMap varmap;

	ResultTestDef const* ti = g_resultTests;
	for (int tidx=1; ti->testname; ++ti,++tidx)
	{
		if (g_verbose)
		{
			std::cerr << "Executing test [" << tidx << "] " << ti->testname << std::endl;
		}
		strus::PatternResultFormatMap fmtmap( ti->formatstring, g_errorhnd);
		strus::analyzer::PatternMatcherResult result = getResultFromDef( varmap, ti->result);

		std::string output = fmtmap.map( result);
		if (output.empty() && g_errorhnd->hasError())
		{
			throw std::runtime_error( g_errorhnd->fetchError());
		}
		checkPatternMatchResult( output.c_str(), ti->expected);
	}
}

int main( int argc, const char* argv[])
{
	int argi = 1;
	for (; argi < argc && argv[argi][0] == '-'; ++argi)
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
		if (g_verbose)
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
		runItemTests();
		runResultTests();
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


