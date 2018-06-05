/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Test of document analysis with a focus of binding terms to ordinal positions
#include "strus/lib/textproc.hpp"
#include "strus/lib/analyzer.hpp"
#include "strus/lib/pattern_termfeeder.hpp"
#include "strus/lib/pattern_test.hpp"
#include "strus/lib/error.hpp"
#include "strus/errorBufferInterface.hpp"
#include "strus/debugTraceInterface.hpp"
#include "strus/queryAnalyzerInstanceInterface.hpp"
#include "strus/queryAnalyzerContextInterface.hpp"
#include "strus/analyzer/documentClass.hpp"
#include "strus/analyzer/functionView.hpp"
#include "strus/textProcessorInterface.hpp"
#include "strus/normalizerFunctionInterface.hpp"
#include "strus/normalizerFunctionInstanceInterface.hpp"
#include "strus/tokenizerFunctionInterface.hpp"
#include "strus/tokenizerFunctionInstanceInterface.hpp"
#include "strus/patternTermFeederInterface.hpp"
#include "strus/patternTermFeederInstanceInterface.hpp"
#include "strus/patternMatcherInterface.hpp"
#include "strus/patternMatcherInstanceInterface.hpp"
#include "strus/analyzerObjectBuilderInterface.hpp"
#include "strus/base/local_ptr.hpp"
#include "strus/base/string_format.hpp"
#include "strus/base/numstring.hpp"
#include "strus/base/fileio.hpp"
#include "strus/base/pseudoRandom.hpp"
#include "strus/reference.hpp"
#include "tree.hpp"
#include <limits>
#include <string>
#include <cstring>
#include <cstdio>
#include <cmath>
#include <stdexcept>
#include <iostream>
#include <sstream>

static strus::ErrorBufferInterface* g_errorhnd = 0;

struct BaseFeature
{
	const char* name;
	const char* tokenizer;
	const char* normalizers[4];
};

struct ConceptFeature
{
	const char* name;
	const char* elements[10];
};

struct TestDescription
{
	const char* name;			//< name of the test
	BaseFeature basefeat;			//< base feature
	ConceptFeature conceptfeats[ 16];	//< {NULL,} terminated list of concept rules
	const char* queryfields[16];		//< NULL terminated list of query fields
	const char* expected;			//< result expected
};

static int lexemid( int conceptidx, int featidx)
{
	return ((conceptidx+1) * 100) + featidx + 1;
}

struct ConceptLexem
{
	int id;
	const char* str;

	ConceptLexem( int id_, const char* str_)
		:id(id_),str(str_){}
	ConceptLexem( const ConceptLexem& o)
		:id(o.id),str(o.str){}
};

static std::vector<ConceptLexem> getConceptLexems( const TestDescription* descr)
{
	std::vector<ConceptLexem> rt;
	const ConceptFeature* ci = descr->conceptfeats;
	for (int cidx=0; ci->name; ++ci,++cidx)
	{
		const char* const* ei = ci->elements;
		for (int eidx=0; *ei; ++ei,++eidx)
		{
			rt.push_back( ConceptLexem( lexemid(cidx,eidx), *ei));
		}
	}
	return rt;
}

static void defineQueryAnalysis( const TestDescription* descr, strus::QueryAnalyzerInstanceInterface* analyzer, strus::TextProcessorInterface* textproc)
{
	// Define BaseFeature:
	const strus::TokenizerFunctionInterface* tokenizer( textproc->getTokenizer( descr->basefeat.tokenizer));
	if (!tokenizer) throw std::runtime_error( g_errorhnd->fetchError());
	std::vector<const strus::NormalizerFunctionInterface*> normalizers;
	const char* const* ni = descr->basefeat.normalizers;
	for (; *ni; ++ni)
	{
		const strus::NormalizerFunctionInterface* normalizer = textproc->getNormalizer( *ni);
		normalizers.push_back( normalizer);
	}
	strus::TokenizerFunctionInstanceInterface* tinst = tokenizer->createInstance( std::vector<std::string>(), textproc);
	if (!tinst) throw std::runtime_error( g_errorhnd->fetchError());

	std::vector<strus::NormalizerFunctionInstanceInterface*> ninstar;
	std::vector<const strus::NormalizerFunctionInterface*>::const_iterator zi = normalizers.begin(), ze = normalizers.end();
	for (; zi != ze; ++zi)
	{
		strus::NormalizerFunctionInstanceInterface* ninst = (*zi)->createInstance( std::vector<std::string>(), textproc);
		if (!ninst) throw std::runtime_error( g_errorhnd->fetchError());
		ninstar.push_back( ninst);
	}
	analyzer->addElement( descr->basefeat.name, "text", tinst, ninstar);

	// Define ConceptFeatures:
	strus::local_ptr<strus::PatternTermFeederInterface> termfeeder( strus::createPatternTermFeeder_default( g_errorhnd));
	if (!termfeeder.get()) throw std::runtime_error( g_errorhnd->fetchError());
	strus::local_ptr<strus::PatternTermFeederInstanceInterface> termfeederinst( termfeeder->createInstance());
	if (!termfeederinst.get()) throw std::runtime_error( g_errorhnd->fetchError());
	termfeederinst->defineLexem( 1, descr->basefeat.name);
	std::vector<ConceptLexem> lexems = getConceptLexems( descr);
	std::vector<ConceptLexem>::const_iterator li = lexems.begin(), le = lexems.end();
	for (; li != le; ++li)
	{
		termfeederinst->defineSymbol( li->id, 1/*lexemid*/, li->str);
	}
	strus::local_ptr<strus::PatternMatcherInterface> matcher( strus::createPatternMatcher_test( g_errorhnd));
	if (!matcher.get()) throw std::runtime_error( g_errorhnd->fetchError());
	strus::local_ptr<strus::PatternMatcherInstanceInterface> matcherinst( matcher->createInstance());
	if (!matcherinst.get()) throw std::runtime_error( g_errorhnd->fetchError());

	ConceptFeature const* ci = descr->conceptfeats;
	for (int cidx=0; ci->name; ++ci,++cidx)
	{
		const char* const* ei = ci->elements;
		int eidx = 0;
		for (; *ei; ++ei,++eidx)
		{
			unsigned int symid = termfeederinst->getSymbol( 1/*lexemid*/, *ei);
			if (!symid) throw std::runtime_error("internal: unknown symbol");
			matcherinst->pushTerm( symid);
		}
		matcherinst->pushExpression( strus::PatternMatcherInstanceInterface::OpSequenceImm,  eidx, eidx+1, 0);
		matcherinst->definePattern( ci->name, std::string()/*formatstring*/, true);
	}
	if (!matcherinst->compile())
	{
		throw std::runtime_error( g_errorhnd->fetchError());
	}
	analyzer->definePatternMatcherPostProc( "pattern", matcherinst.get(), termfeederinst.get());
	matcherinst.release();
	termfeederinst.release();

	analyzer->addElementFromPatternMatch( "pattern", "pattern", std::vector<strus::NormalizerFunctionInstanceInterface*>());
	analyzer->declareTermPriority( "pattern", 1);
}

static std::string queryTermExpression_tostring( const strus::analyzer::QueryTermExpression* res)
{
	std::ostringstream out;
	std::vector<strus::analyzer::QueryTermExpression::Instruction>::const_iterator
		ii = res->instructions().begin(),
		ie = res->instructions().end();
	for (; ii != ie; ++ii)
	{
		switch (ii->opCode())
		{
			case strus::analyzer::QueryTermExpression::Instruction::Operator:
				throw std::runtime_error("operator unexpected in query analysis result");
			case strus::analyzer::QueryTermExpression::Instruction::Term:
			{
				const strus::analyzer::QueryTerm& term = res->term( ii->idx());
				out << term.type() << " '" << term.value() << "' [" << term.len() << "]" << std::endl;
				break;
			}
		}
	}
	return out.str();
}

static void printDebugTraceMessages( std::ostream& out)
{
	strus::DebugTraceInterface* dbgtrace = g_errorhnd->debugTrace();
	if (dbgtrace)
	{
		std::vector<strus::DebugTraceMessage> msglist = dbgtrace->fetchMessages();
		std::vector<strus::DebugTraceMessage>::const_iterator mi = msglist.begin(), me = msglist.end();
		for (; mi != me; ++mi)
		{
			out << mi->typeName() << " " << mi->component() << " " << mi->id() << " [" << mi->content() << "]" << std::endl;
		}
	}
}

static void runTest( const TestDescription* descr, strus::TextProcessorInterface* textproc)
{
	std::cerr << "run test " << descr->name << std::endl;
	strus::local_ptr<strus::QueryAnalyzerInstanceInterface> analyzer( strus::createQueryAnalyzer( g_errorhnd));
	if (!analyzer.get()) throw std::runtime_error( g_errorhnd->fetchError());
	defineQueryAnalysis( descr, analyzer.get(), textproc);
	strus::local_ptr<strus::QueryAnalyzerContextInterface> ctx( analyzer->createContext());
	const char* const* qi = descr->queryfields;
	for (int qidx=0; *qi; ++qi,++qidx)
	{
		ctx->putField( qidx+1, "text", *qi);
	}
	strus::analyzer::QueryTermExpression res = ctx->analyze();
	std::string resultstr = queryTermExpression_tostring( &res);
	if (g_errorhnd->hasError())
	{
		throw std::runtime_error( g_errorhnd->fetchError());
	}
	printDebugTraceMessages( std::cerr);
	if (resultstr != descr->expected)
	{
		std::cout << "RESULT\n" << resultstr << std::endl;
		std::cout << "EXPECTED\n" << descr->expected << std::endl;
		throw std::runtime_error( "test result not as expected");
	}
}

static const TestDescription test1 = {
	"1",
	{"word","word",{"lc",0}},
	{
		{"bla_dup", {"bla","bla",0}},
		{0}
	},
	{"bla bla bla",0},
	"pattern 'bla_dup' [2]\n"
	"pattern 'bla_dup' [2]\n"
};

static const TestDescription test2 = {
	"2",
	{"word","word",{"lc",0}},
	{
		{"bla_dup", {"bla","bla",0}},
		{0}
	},
	{"bla bla","bla",0},
	"pattern 'bla_dup' [2]\n"
	"word 'bla' [1]\n"
};

static const TestDescription test3 = {
	"3",
	{"word","word",{"lc",0}},
	{
		{"bla_dup", {"bla","bla",0}},
		{"hup_tri", {"hup","hup","hup",0}},
		{0}
	},
	{"bli bla blu bla bla bla hup hup hup hup hup bla bli","bla bla","bla","hup hup bla hup",0},
	"word 'bli' [1]\n"
	"word 'bla' [1]\n"
	"word 'blu' [1]\n"
	"pattern 'bla_dup' [2]\n"
	"pattern 'bla_dup' [2]\n"
	"pattern 'hup_tri' [3]\n"
	"pattern 'hup_tri' [3]\n"
	"pattern 'hup_tri' [3]\n"
	"word 'bla' [1]\n"
	"word 'bli' [1]\n"
	"pattern 'bla_dup' [2]\n"
	"word 'bla' [1]\n"
	"word 'hup' [1]\n"
	"word 'hup' [1]\n"
	"word 'bla' [1]\n"
	"word 'hup' [1]\n"
	""
};

int main( int argc, const char* argv[])
{
	try
	{
		bool verbose = false;
		if (argc > 1 && 0==std::strcmp(argv[1],"-V"))
		{
			verbose = true;
		}
		strus::DebugTraceInterface* dbgtrace = NULL;
		if (verbose)
		{
			dbgtrace = strus::createDebugTrace_standard( 2);
			if (!dbgtrace) throw std::bad_alloc();
			dbgtrace->enable( "analyzer");
			dbgtrace->enable( "pattern");
		}
		g_errorhnd = strus::createErrorBuffer_standard( 0, 2, dbgtrace);
		if (!g_errorhnd)
		{
			throw std::runtime_error("failed to create error buffer object");
		}
		strus::local_ptr<strus::TextProcessorInterface> textproc( createTextProcessor( g_errorhnd));
		if (!textproc.get()) throw std::runtime_error( g_errorhnd->fetchError());

		if (g_errorhnd->hasError())
		{
			throw std::runtime_error( g_errorhnd->fetchError());
		}
		runTest( &test1, textproc.get());
		runTest( &test2, textproc.get());
		runTest( &test3, textproc.get());
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



