/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Test of document analysis with a focus of binding terms to ordinal positions
#include "strus/lib/analyzer_objbuild.hpp"
#include "strus/lib/error.hpp"
#include "strus/errorBufferInterface.hpp"
#include "strus/segmenterInterface.hpp"
#include "strus/segmenterInstanceInterface.hpp"
#include "strus/segmenterContextInterface.hpp"
#include "strus/documentAnalyzerInterface.hpp"
#include "strus/documentAnalyzerContextInterface.hpp"
#include "strus/analyzer/documentClass.hpp"
#include "strus/textProcessorInterface.hpp"
#include "strus/aggregatorFunctionInterface.hpp"
#include "strus/aggregatorFunctionInstanceInterface.hpp"
#include "strus/normalizerFunctionInterface.hpp"
#include "strus/normalizerFunctionInstanceInterface.hpp"
#include "strus/tokenizerFunctionInterface.hpp"
#include "strus/tokenizerFunctionInstanceInterface.hpp"
#include "strus/analyzerObjectBuilderInterface.hpp"
#include "strus/base/fileio.hpp"
#include "strus/base/local_ptr.hpp"
#include "strus/analyzer/positionBind.hpp"
#include "strus/analyzer/featureOptions.hpp"
#include <memory>
#include <algorithm>
#include <string>
#include <cstring>
#include <stdexcept>
#include <iostream>
#include <sstream>

#undef STRUS_LOWLEVEL_DEBUG

static strus::ErrorBufferInterface* g_errorhnd = 0;

static void printUsage( int argc, const char* argv[])
{
	std::cerr << "usage: " << argv[0] << " <inputfile> <expectedfile>" << std::endl;
	std::cerr << "<inputfile> = file containing documents to analyze" << std::endl;
	std::cerr << "<expectedfile> = file containing the result expected" << std::endl;
}


struct ConfigItem
{
	enum Type {Attribute,SearchIndex,ForwardIndex};
	Type type;
	const char* name;
	const char* normalizers[10];
	const char* tokenizer;
	const char* path;
	int posbind;
};

struct FuncDef
{
	std::string name;
	std::vector<std::string> args;

	FuncDef( const char* def)
	{
		char const* si = def;
		char const* sn = std::strchr( si, ' ');
		if (!sn) sn = std::strchr( si, '\0');
		name.append( si, sn-si);
		if (*sn)
		{
			for (si=sn+1,sn=std::strchr(si,' '); sn; si=sn+1,sn=std::strchr(si,' '))
			{
				args.push_back( std::string( si, sn-si));
			}
			args.push_back( si);
		}
	}
};

static void loadAnalyzerConfig( strus::DocumentAnalyzerInterface* analyzer, const strus::TextProcessorInterface* textproc)
{
	static const ConfigItem config[32] =
	{
		{ConfigItem::Attribute,"title",{"orig",0},"content","/doc/title()",0},
		{ConfigItem::SearchIndex,"para",{"orig",0},"content","/doc/text",0},
		{ConfigItem::SearchIndex,"para",{"empty",0},"content","/doc/title",0},
		{ConfigItem::SearchIndex,"tag",{"orig",0},"word","/doc/text/tag()",0},
		{ConfigItem::SearchIndex,"annot",{"orig",0},"word","/doc/text/annot",0},
		{ConfigItem::SearchIndex,"aid",{"orig",0},"word","/doc/text/annot@id",0},
		{ConfigItem::SearchIndex,"stem",{"convdia de","stem de","lc",0},"word","/doc/text//()",0},
		{ConfigItem::SearchIndex,"sent",{"orig",0},"punctuation de","/doc/text//()",0},
		{ConfigItem::ForwardIndex,"orig",{"orig",0},"word","//text//()",0},
		{ConfigItem::Attribute,0,{0},0,0}
	};
	analyzer->defineSubDocument( "doc", "/doc");
	const strus::AggregatorFunctionInterface* aggf = textproc->getAggregator( "count");
	strus::local_ptr<strus::AggregatorFunctionInstanceInterface> aggfi( aggf->createInstance( std::vector<std::string>( 1,"stem")));
	analyzer->defineAggregatedMetaData( "doclen", aggfi.get());
	(void)aggfi.release();

	ConfigItem const* ci = config;
	for (; ci->name; ++ci)
	{
		FuncDef tdef( ci->tokenizer);
		const strus::TokenizerFunctionInterface* tk = textproc->getTokenizer( tdef.name);
		if (!tk) throw std::runtime_error( std::string("unknown tokenizer: '") + tdef.name + "'");
		strus::local_ptr<strus::TokenizerFunctionInstanceInterface> tki( tk->createInstance( tdef.args, textproc));
		std::vector<strus::NormalizerFunctionInstanceInterface*> normalizers;
		std::size_t ni = 0;
		for (; ci->normalizers[ni]; ++ni)
		{
			FuncDef ndef( ci->normalizers[ni]);
			const strus::NormalizerFunctionInterface* nm = textproc->getNormalizer( ndef.name);
			if (!nm) throw std::runtime_error( std::string("unknown normalizer: '") + ndef.name + "'");
			strus::local_ptr<strus::NormalizerFunctionInstanceInterface> nmi( nm->createInstance( ndef.args, textproc));
			if (!nmi.get())
			{
				std::vector<strus::NormalizerFunctionInstanceInterface*>::iterator zi = normalizers.begin(), ze = normalizers.end();
				for (; zi != ze; ++zi)
				{
					delete *zi;
				}
				throw std::runtime_error("failed to create normalizers");
			}
			normalizers.push_back( nmi.release());
		}
		strus::analyzer::FeatureOptions opt;
		if (ci->posbind > 0)
		{
			opt.definePositionBind( strus::analyzer::BindSuccessor);
		}
		else if (ci->posbind < 0)
		{
			opt.definePositionBind( strus::analyzer::BindPredecessor);
		}
		switch (ci->type)
		{
			case ConfigItem::Attribute:
				analyzer->defineAttribute( ci->name, ci->path, tki.release(), normalizers);
				break;
			case ConfigItem::SearchIndex:
				analyzer->addSearchIndexFeature( ci->name, ci->path, tki.release(), normalizers, opt);
				break;
			case ConfigItem::ForwardIndex:
				analyzer->addForwardIndexFeature( ci->name, ci->path, tki.release(), normalizers, opt);
				break;
		}
	}
}

int main( int argc, const char* argv[])
{
	if (argc <= 1 || std::strcmp( argv[1], "-h") == 0 || std::strcmp( argv[1], "--help") == 0)
	{
		printUsage( argc, argv);
		return 0;
	}
	else if (argc < 3)
	{
		std::cerr << "ERROR too few parameters" << std::endl;
		printUsage( argc, argv);
		return 1;
	}
	else if (argc > 3)
	{
		std::cerr << "ERROR too many parameters" << std::endl;
		printUsage( argc, argv);
		return 1;
	}
	try
	{
		g_errorhnd = strus::createErrorBuffer_standard( 0, 2, NULL/*debug trace interface*/);
		if (!g_errorhnd)
		{
			throw std::runtime_error("failed to create error buffer object");
		}
		std::string inputfile( argv[1]);
		std::string expectedfile( argv[2]);
		std::string outputfile;
		char const* xi = argv[2];
		char const* xn = std::strchr( xi,'.');
		if (xn)
		{
			for (; xn; xi=xn+1,xn=std::strchr(xi,'.')){}
			outputfile.append( std::string( argv[2], xi-argv[2]) + "out");
		}
		else
		{
			outputfile.append( std::string( argv[2]) + ".out");
		}
		strus::local_ptr<strus::AnalyzerObjectBuilderInterface> objbuild(
			strus::createAnalyzerObjectBuilder_default( g_errorhnd));
		const strus::TextProcessorInterface* textproc = objbuild->getTextProcessor();
		const strus::SegmenterInterface* segmenter = textproc->getSegmenterByName( "textwolf");
		strus::local_ptr<strus::DocumentAnalyzerInterface> analyzer( objbuild->createDocumentAnalyzer( segmenter));
		loadAnalyzerConfig( analyzer.get(), textproc);

		std::string inputsrc;
		unsigned int ec;
		ec = strus::readFile( inputfile, inputsrc);
		if (ec) throw std::runtime_error( std::string("error reading input file ") + inputfile + ": " + ::strerror(ec));
		std::string expectedsrc;
		ec = strus::readFile( expectedfile, expectedsrc);
		if (ec) throw std::runtime_error( std::string("error reading expected file ") + expectedfile + ": " + ::strerror(ec));

		strus::analyzer::DocumentClass documentClass( "application/xml", "UTF-8");
		std::ostringstream output;
		strus::local_ptr<strus::DocumentAnalyzerContextInterface> analyzerctx( analyzer->createContext( documentClass));
		analyzerctx->putInput( inputsrc.c_str(), inputsrc.size(), true);
		strus::analyzer::Document doc;
		while (analyzerctx->analyzeNext( doc))
		{
			output << "DOC " << doc.subDocumentTypeName() << std::endl;
			std::vector<strus::analyzer::DocumentAttribute>::const_iterator
				ai = doc.attributes().begin(), ae = doc.attributes().end();
			for (; ai != ae; ++ai)
			{
				output << "Attribute " << ai->name() << " = '" << ai->value() << "'" << std::endl;
			}
			std::vector<strus::analyzer::DocumentMetaData>::const_iterator
				mi = doc.metadata().begin(), me = doc.metadata().end();
			for (; mi != me; ++mi)
			{
				output << "MetaData " << mi->name() << " = '" << mi->value().tostring().c_str() << "'" << std::endl;
			}
			std::vector<strus::analyzer::DocumentTerm> searchIndexTerms = doc.searchIndexTerms();
			std::sort( searchIndexTerms.begin(), searchIndexTerms.end());
			std::vector<strus::analyzer::DocumentTerm>::const_iterator
				ti = searchIndexTerms.begin(), te = searchIndexTerms.end();
			for (; ti != te; ++ti)
			{
				output << "SearchTerm term " << ti->type() << " '" << ti->value() << "' at " << ti->pos() << std::endl;
			}
			std::vector<strus::analyzer::DocumentTerm> forwardIndexTerms = doc.forwardIndexTerms();
			std::sort( forwardIndexTerms.begin(), forwardIndexTerms.end());
			ti = forwardIndexTerms.begin(), te = forwardIndexTerms.end();
			for (; ti != te; ++ti)
			{
				output << "ForwardIndex term " << ti->type() << " '" << ti->value() << "' at " << ti->pos() << std::endl;
			}
		}
#ifdef STRUS_LOWLEVEL_DEBUG
		std::cout << output.str();
#endif
		if (g_errorhnd->hasError())
		{
			throw std::runtime_error( g_errorhnd->fetchError());
		}

		ec = strus::writeFile( outputfile, output.str());
		if (ec) throw std::runtime_error( std::string("error writing output file ") + outputfile + ": " + ::strerror(ec));

		if (output.str() != expectedsrc)
		{
			throw std::runtime_error("output not as expected");
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


