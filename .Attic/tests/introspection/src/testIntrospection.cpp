/*
 * Copyright (c) 2018 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "strus/lib/analyzer.hpp"
#include "strus/lib/textproc.hpp"
#include "strus/lib/detector_std.hpp"
#include "strus/lib/error.hpp"
#include "strus/lib/normalizer_charconv.hpp"
#include "strus/lib/normalizer_dateconv.hpp"
#include "strus/lib/normalizer_dictmap.hpp"
#include "strus/lib/normalizer_ngram.hpp"
#include "strus/lib/normalizer_regex.hpp"
#include "strus/lib/normalizer_snowball.hpp"
#include "strus/lib/normalizer_trim.hpp"
#include "strus/lib/normalizer_wordjoin.hpp"
#include "strus/lib/segmenter_textwolf.hpp"
#include "strus/lib/segmenter_cjson.hpp"
#include "strus/lib/segmenter_plain.hpp"
#include "strus/lib/segmenter_tsv.hpp"
#include "strus/lib/tokenizer_punctuation.hpp"
#include "strus/lib/tokenizer_word.hpp"
#include "strus/lib/tokenizer_regex.hpp"
#include "strus/lib/tokenizer_textcat.hpp"
#include "strus/lib/aggregator_set.hpp"
#include "strus/lib/aggregator_vsm.hpp"
#include "strus/errorBufferInterface.hpp"
#include "strus/documentAnalyzerInterface.hpp"
#include "strus/normalizerFunctionInstanceInterface.hpp"
#include "strus/normalizerFunctionInterface.hpp"
#include "strus/segmenterContextInterface.hpp"
#include "strus/segmenterInstanceInterface.hpp"
#include "strus/segmenterInterface.hpp"
#include "strus/textProcessorInterface.hpp"
#include "strus/tokenizerFunctionInstanceInterface.hpp"
#include "strus/tokenizerFunctionInterface.hpp"
#include "strus/aggregatorFunctionInstanceInterface.hpp"
#include "strus/aggregatorFunctionInterface.hpp"
#include "strus/analyzer/token.hpp"
#include "strus/analyzer/positionBind.hpp"
#include "strus/analyzer/featureOptions.hpp"
#include "strus/introspectionInterface.hpp"
#include "strus/base/string_format.hpp"
#include "strus/base/stdint.h"
#include "strus/base/local_ptr.hpp"
#include "strus/base/pseudoRandom.hpp"
#include "private/internationalization.hpp"
#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <memory>
#include <cstdarg>
#include <cstdio>
#include <cstring>

#define STRUS_LOWLEVEL_DEBUG

static strus::ErrorBufferInterface* g_errorhnd = NULL;

struct FunctionDescription
{
	const char* name;
	const char* arg[ 6];
};

struct FeatureDescription
{
	const char* featclass;
	const char* type;
	const char* select;
	const FunctionDescription tokenizer;
	const FunctionDescription normalizer[6];
	const char* posopt;
};

static bool isEqual( const char* name, const char* oth)
{
	return 0==std::strcmp( name, oth);
}

static std::vector<std::string> getArgs( char const* const* arg)
{
	std::vector<std::string> rt;
	std::size_t ai=0;
	for (; arg[ai]; ++ai)
	{
		rt.push_back( arg[ai]);
	}
	return rt;
}

static void addFeature( strus::DocumentAnalyzerInterface* analyzer, const strus::TextProcessorInterface* textproc, const FeatureDescription* descr)
{
	if (isEqual( descr->featclass, "aggregator"))
	{
		const strus::AggregatorFunctionInterface* func = textproc->getAggregator( descr->tokenizer.name);
		if (!func) throw std::runtime_error( g_errorhnd->fetchError());
		strus::AggregatorFunctionInstanceInterface* instance = func->createInstance( getArgs( descr->tokenizer.arg));
		if (!instance) throw std::runtime_error( g_errorhnd->fetchError());
		analyzer->defineAggregatedMetaData( descr->type, instance);
		return;
	}
	else if (isEqual( descr->featclass, "subdoc"))
	{
		analyzer->defineSubDocument( descr->type, descr->select);
		return;
	}
	else if (isEqual( descr->featclass, "subcontent"))
	{
		char const* charset = std::strchr( descr->type, ';');
		std::string doctype = charset ? std::string( descr->type, charset-descr->type) : std::string( descr->type);
		if (!charset) charset = "UTF-8";
		strus::analyzer::DocumentClass docClass( doctype, charset);
		analyzer->defineSubContent( descr->select, docClass);
		return;
	}
	else
	{
		std::vector<strus::NormalizerFunctionInstanceInterface*> normalizers;
		strus::TokenizerFunctionInstanceInterface* tokenizer;
		FunctionDescription const* ni = descr->normalizer;
		for (; ni->name; ++ni)
		{
			const strus::NormalizerFunctionInterface* normalizer = textproc->getNormalizer( ni->name);
			if (!normalizer) throw std::runtime_error( g_errorhnd->fetchError());
	
			strus::NormalizerFunctionInstanceInterface* instance = normalizer->createInstance( getArgs( ni->arg), textproc);
			if (!instance) throw std::runtime_error( strus::string_format("failed to create normalizer '%s' instance: %s", ni->name, g_errorhnd->fetchError()));

			normalizers.push_back( instance);
		}
		FunctionDescription const* td = &descr->tokenizer;
		const strus::TokenizerFunctionInterface* tokenizer_type = textproc->getTokenizer( td->name);
		if (!tokenizer_type) throw std::runtime_error( g_errorhnd->fetchError());
		tokenizer = tokenizer_type->createInstance( getArgs( td->arg), textproc);
		strus::analyzer::FeatureOptions fopt;
		if (descr->posopt)
		{
			if (isEqual(descr->posopt,"content")) fopt.definePositionBind( strus::analyzer::BindContent);
			else if (isEqual(descr->posopt,"succ")) fopt.definePositionBind( strus::analyzer::BindSuccessor);
			else if (isEqual(descr->posopt,"pred")) fopt.definePositionBind( strus::analyzer::BindPredecessor);
			else if (isEqual(descr->posopt,"unique")) fopt.definePositionBind( strus::analyzer::BindUnique);
			else throw std::runtime_error("unknown position bind option");
		}
		if (isEqual( descr->featclass, "search"))
		{
			analyzer->addSearchIndexFeature( descr->type, descr->select, tokenizer, normalizers, fopt);
		}
		else if (isEqual( descr->featclass, "forward"))
		{
			analyzer->addForwardIndexFeature( descr->type, descr->select, tokenizer, normalizers, fopt);
		}
		else if (isEqual( descr->featclass, "metadata"))
		{
			analyzer->defineMetaData( descr->type, descr->select, tokenizer, normalizers);
		}
		else if (isEqual( descr->featclass, "attribute"))
		{
			analyzer->defineAttribute( descr->type, descr->select, tokenizer, normalizers);
		}
		else
		{
			throw std::runtime_error("unknown feature class");
		}
	}
}


static void testInitAnalyzer( strus::DocumentAnalyzerInterface* analyzer, const strus::TextProcessorInterface* textproc)
{
	static const FeatureDescription definitions[32] = 
	{
		{"search","punct","/doc/text()", {"punctuation",{"de",0}}, { {"empty",{0}},{0} }, "pred"},
		{"search","title","/doc/title()", {"content",{0}}, { {"trim",{0}},{"convdia",{"de",0}},{"uc",{0}},{0} }, (const char*)0},
		{"search","stem","/doc/text()", {"word",{0}}, { {"stem",{"de",0}},{"lc",{0}},{0} }, (const char*)0},
		{"search","catg","/doc/catg()", {"word",{0}}, { {"dictmap",{"testdict.txt","0",0}},{0} }, (const char*)0},
		{"search","ngram","/doc/text()", {"word",{0}}, { {"ngram",{"RoundRobin","RoundRobin","WithStart",0}},{"uc",{0}},{0} }, (const char*)0},
		{"forward","orig","/doc/text()", {"split",{0}}, { {"orig",{0}},{0} }, (const char*)0},
		{"forward","orig_de","/doc/text()", {"textcat",{"conf.txt","de",0}}, { {"text",{0}},{"wordjoin",{0}},{0} }, (const char*)0},
		{"metadata","date","/doc/date()", {"regex",{"[0-9]{2,4}[/][0-9]{2,2}[/][0-9]{2,2}",0}}, { {"regex",{"[0-9]{2,4}[/][0-9]{2,2}[/][0-9]{2,2}","$0",0}},{"date2int",{0}},{0} }, (const char*)0},
		{"aggregator", "doclen", "", {"count", {"stem",0}}, {{0}}, (const char*)0},
		{"subdoc", "doc", "/doc", {0}, {{0}}, (const char*)0},
		{"subcontent", "application/json", "/doc", {0}, {{0}}, (const char*)0},
		{0,0,0,{0},{{0}},0}
	};
	FeatureDescription const* fi = definitions;
	for (; fi->featclass; ++fi)
	{
#ifdef STRUS_LOWLEVEL_DEBUG
		std::cerr << strus::string_format( "init class '%s', type '%s', select '%s'", fi->featclass, fi->type, fi->select) << std::endl;
#endif
		addFeature( analyzer, textproc, fi);
	}
}

static void printUsage( int argc, const char* argv[])
{
	std::cerr << "usage: " << argv[0] << " <resourcedir> <textcatdir>" << std::endl;
	std::cerr << "<resourcedir> = location of the resources to load" << std::endl;
	std::cerr << "<textcatdir> = location of the resources for text categorization" << std::endl;
}

static void printIntrospection( std::ostream& out, strus::IntrospectionInterface* introspection, int indentcnt)
{
	std::string indentstr( indentcnt*2, ' ');
	std::string value = introspection->value();
	if (!value.empty())
	{
		out << indentstr << "[" << value << "]" << std::endl;
	}
	std::vector<std::string> list( introspection->list());
	std::vector<std::string>::const_iterator li = list.begin(), le = list.end();
	for (; li != le; ++li)
	{
		strus::local_ptr<strus::IntrospectionInterface> chld( introspection->open( *li));
		if (chld.get())
		{
			out << indentstr << *li << ":" << std::endl;
			printIntrospection( out, chld.get(), indentcnt+1);
		}
		else
		{
			out << indentstr << *li << std::endl;
		}
	}
}

int main( int argc, const char* argv[])
{
	try
	{
		if (argc <= 2 || std::strcmp( argv[1], "-h") == 0 || std::strcmp( argv[1], "--help") == 0)
		{
			printUsage( argc, argv);
			return 0;
		}
		const char* resourcePath = argv[1];
		const char* textcatPath = argv[2];
		
		g_errorhnd = strus::createErrorBuffer_standard( 0, 2, NULL/*debug trace interface*/ );
		if (!g_errorhnd) throw std::runtime_error("failed to create error buffer object");

		strus::local_ptr<strus::TextProcessorInterface> textproc( strus::createTextProcessor( g_errorhnd));
		if (!textproc.get()) throw std::runtime_error( g_errorhnd->fetchError());
		textproc->addResourcePath( resourcePath);
		textproc->addResourcePath( textcatPath);
		strus::local_ptr<strus::SegmenterInterface> segmenter( strus::createSegmenter_textwolf( g_errorhnd));
		if (!segmenter.get()) throw std::runtime_error( g_errorhnd->fetchError());
		strus::local_ptr<strus::DocumentAnalyzerInterface> analyzer( strus::createDocumentAnalyzer( textproc.get(), segmenter.get(), strus::analyzer::SegmenterOptions(), g_errorhnd));
		if (!analyzer.get()) throw std::runtime_error( g_errorhnd->fetchError());

		testInitAnalyzer( analyzer.get(), textproc.get());
		strus::local_ptr<strus::IntrospectionInterface> introspection( analyzer->createIntrospection());
		printIntrospection( std::cout, introspection.get(), 0);

		delete g_errorhnd;
		std::cerr << "done" << std::endl;
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


