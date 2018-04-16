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
#include "strus/lib/error.hpp"
#include "strus/errorBufferInterface.hpp"
#include "strus/queryAnalyzerInterface.hpp"
#include "strus/queryAnalyzerContextInterface.hpp"
#include "strus/analyzer/documentClass.hpp"
#include "strus/textProcessorInterface.hpp"
#include "strus/normalizerFunctionInterface.hpp"
#include "strus/normalizerFunctionInstanceInterface.hpp"
#include "strus/tokenizerFunctionInterface.hpp"
#include "strus/tokenizerFunctionInstanceInterface.hpp"
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

#undef STRUS_LOWLEVEL_DEBUG

static strus::ErrorBufferInterface* g_errorhnd = 0;

static void printUsage( int argc, const char* argv[])
{
	std::cerr << "usage: " << argv[0] << "<outputdir> <noftests> <treesize>" << std::endl;
	std::cerr << "<outputdir> = directory for output" << std::endl;
	std::cerr << "<noftests> = number of tests to run" << std::endl;
	std::cerr << "<treesize> = maximum size of tree in each test" << std::endl;
}

class QueryItem
{
public:
	std::string type;
	std::string value;
	int range;
	int cardinality;
	int len;

	QueryItem()
		:type(),value(),range(-1),cardinality(-1),len(-1){}
	QueryItem( const std::string& type_, const std::string& value_, int len_)
		:type(type_),value(value_),range(-1),cardinality(-1),len(len_){}
	QueryItem( const std::string& op_, int range_, int cardinality_)
		:type(),value(op_),range(range_),cardinality(cardinality_),len(-1){}
	QueryItem( const QueryItem& o)
		:type(o.type),value(o.value),range(o.range),cardinality(o.cardinality),len(o.len){}

	bool isOperator() const
	{
		return type.empty();
	}

	friend std::ostream& operator<<( std::ostream& os, const QueryItem& itm);
};

std::ostream& operator<<( std::ostream& os, const QueryItem& itm)
{
	if (itm.isOperator())
	{
		os << "[" << itm.value << " " << itm.range << " " << itm.cardinality << "]";
	}
	else
	{
		os << itm.type << " '" << itm.value << "' " << itm.len;
	}
	return os;
}

typedef strus::test::TreeNode<QueryItem> QueryTree;

static strus::PseudoRandom g_random;

static int parseInt( const char* intstr)
{
	strus::NumParseError errcode = strus::NumParseOk;

	int64_t rt = strus::intFromString( intstr, std::strlen(intstr), std::numeric_limits<int>::max(), errcode);
	if (errcode != strus::NumParseOk)
	{
		throw strus::numstring_exception( errcode);
	}
	return (int)rt;
}

static std::string fieldTypeName( int fieldTypeNo)
{
	char buf[ 64];
	std::snprintf( buf, sizeof(buf), "F%d", fieldTypeNo+1);
	return std::string(buf);
}

static int fieldTypeValue( const std::string& fieldtype)
{
	if (fieldtype.empty() || fieldtype.at(0) != 'F') throw std::runtime_error( strus::string_format( "invalid field type '%s'", fieldtype.c_str()));
	return parseInt( fieldtype.c_str() + 1);
}

static std::string randomFieldType( int max)
{
	return fieldTypeName( g_random.get( 0, max));
}

static std::string randomFieldValue( int max)
{
	char buf[ 64];
	std::snprintf( buf, sizeof(buf), "%d", g_random.get( 1, max+1));
	std::string rt( buf);
	while (g_random.get( 0, 4) == 0)
	{
		rt.append( std::string( g_random.get( 1, g_random.get( 2, g_random.get( 3, 8))), ' '));
		std::snprintf( buf, sizeof(buf), "%d", g_random.get( 1, max+1));
		rt.append( buf);
	}
	return rt;
}

static std::string randomOp( int max)
{
	char buf[ 64];
	std::snprintf( buf, sizeof(buf), "$%d", g_random.get( 1, max+1));
	return std::string(buf);
}

enum {
	ImplicitSeqGroupId=0x10000	//< group id for implicit sequence
};

static QueryItem randomQueryElement( int typemax, int valuemax)
{
	int len = g_random.get( 0, g_random.get( 1, g_random.get( 2, 7)));
	QueryItem rt( randomFieldType( typemax), randomFieldValue( valuemax), len);
	return rt;
}

static QueryItem randomQueryOp( int opmax, int rangehint, int maxcardinality)
{
	int range = (g_random.get( 0, 2) == 0) ? 0 : g_random.get( rangehint, rangehint*3);
	int cardinality = (g_random.get( 0, 2) == 0) ? 0 : g_random.get( 0, maxcardinality);
	return QueryItem( randomOp( opmax), range, cardinality);
}

static strus::QueryAnalyzerContextInterface::GroupBy getOpGroup( const QueryItem& itm)
{
	char const* opidstr = itm.value.c_str();
	if (itm.isOperator() && opidstr[0] == '$')
	{
		int opidnum = strus::numstring_conv::toint( opidstr+1, std::numeric_limits<int>::max());
		switch ((opidnum-1) % 3)
		{
			case 0: return strus::QueryAnalyzerContextInterface::GroupByPosition;
			case 1: return strus::QueryAnalyzerContextInterface::GroupAll;
			case 2: return strus::QueryAnalyzerContextInterface::GroupEvery;
		}
		return strus::QueryAnalyzerContextInterface::GroupUnique;//... never get here, just avoid warning
	}
	else
	{
		throw std::logic_error( strus::string_format( "illegal call of '%s'", "getOpGroup"));
	}
}

static QueryTree randomQueryTree( int size, int childmax, int depth, int opmax, int typemax, int valuemax)
{
	if (depth > 0 && g_random.get( 0, g_random.get( 1, depth+1) + 1) != 0)
	{
		int nof_chld = g_random.get( 1, childmax + 1);
		size -= nof_chld + 1;
		if (size <= 0)
		{
			return QueryTree( randomQueryElement( typemax, valuemax));
		}
		else
		{
			QueryTree rt( randomQueryOp( opmax, nof_chld, nof_chld));
			for (int ni=0; ni<nof_chld; ++ni)
			{
				rt.addChild( randomQueryTree( size, childmax, depth-1, opmax, typemax, valuemax));
			}
			return rt;
		}
	}
	else
	{
		return QueryTree( randomQueryElement( typemax, valuemax));
	}
}

static std::string normalizeMultiplication( const char* src, std::size_t srcsize, int factor)
{
	int val = strus::numstring_conv::toint( src, srcsize, std::numeric_limits<int>::max());
	char numbuf[ 64];
	std::snprintf( numbuf, sizeof(numbuf), "%d", val * factor);
	try
	{
		std::string rt( numbuf);
		return rt;
	}
	catch (const std::runtime_error&)
	{
		g_errorhnd->report( strus::ErrorCodeOutOfMem, "out of memory in 'multiply' normalizer");
		return std::string();
	}
}

static std::string normalizeMultiplication( const std::string& src, int factor)
{
	return normalizeMultiplication( src.c_str(), src.size(), factor);
}

class MultiplyNormalizerFunctionInstance :public strus::NormalizerFunctionInstanceInterface
{
public:
	MultiplyNormalizerFunctionInstance( int factor_)
		:m_factor(factor_){}
	virtual ~MultiplyNormalizerFunctionInstance(){}

	virtual std::string normalize( const char* src, std::size_t srcsize) const
	{
		return normalizeMultiplication( src, srcsize, m_factor);
	}

private:
	int m_factor;
};

static int charArrayLength( const char** ar)
{
	char const** ai = ar;
	int aidx = 0;
	for (; *ai; ++ai,++aidx){}
	return aidx;
}

static void defineQueryAnalysis( strus::QueryAnalyzerInterface* qana, strus::TextProcessorInterface* textproc, int typemax, const char** desttypes)
{
	int didx = charArrayLength( desttypes);
	if (!didx) throw std::runtime_error("empty feature type definitions");

	int ti=1, te=typemax+1;
	for (; ti < te; ++ti)
	{
		std::string termtype = desttypes[ ti % didx];
		std::string fieldtype = fieldTypeName( ti-1);
		const strus::TokenizerFunctionInterface* tokenizertype = textproc->getTokenizer( "split");
		if (!tokenizertype) throw std::runtime_error( g_errorhnd->fetchError());
		strus::Reference<strus::TokenizerFunctionInstanceInterface> tokenizer( tokenizertype->createInstance( std::vector<std::string>(), textproc));
		if (!tokenizer.get()) throw std::runtime_error( g_errorhnd->fetchError());
		std::vector<strus::NormalizerFunctionInstanceInterface*> normalizers;
		normalizers.push_back( new MultiplyNormalizerFunctionInstance( ti));

		qana->addElement( termtype, fieldtype, tokenizer.get(), normalizers);
#ifdef STRUS_LOWLEVEL_DEBUG
		std::cerr << strus::string_format( "Define query element type '%s' field '%s' normalizer * %d\n", termtype.c_str(), fieldtype.c_str(), ti);
#endif
		tokenizer.release();
	}
}

struct QueryIterNode
{
	const QueryTree* node;
	std::vector<int> fields;

	QueryIterNode( const QueryTree* node_)
		:node(node_),fields(){}
	QueryIterNode( const QueryIterNode& o)
		:node(o.node),fields(o.fields){}
};

static std::vector<const QueryTree*> buildAnalyzerQueryTree( strus::QueryAnalyzerContextInterface* ctx, const QueryTree& qt)
{
	std::vector<const QueryTree*> opnodear;
	unsigned int fieldnocnt = 0;
	std::vector<QueryIterNode> stk;
	stk.push_back( &qt);
	do
	{
		if (!stk.back().node)
		{
			stk.pop_back();
			if (!stk.empty() && stk.back().node->item.isOperator() && !stk.back().fields.empty())
			{
				// Now as we have parsed all children of this operator node, we can define its grouping operation in the analyzer:
				const QueryTree* nd = stk.back().node;
				strus::QueryAnalyzerContextInterface::GroupBy groupBy = getOpGroup( nd->item);
				unsigned int groupid = opnodear.size()+1;
				opnodear.push_back( nd);
				ctx->groupElements( groupid, stk.back().fields, groupBy, true);
				int fieldno = stk.back().fields.at(0);

				// Go to the next element and add one involved fieldno to the parents context
				//	(a field already grouped forwards the next grouping operation to its parent):
				stk.back().node = stk.back().node->next;
				if (stk.size() > 1)
				{
					stk[ stk.size()-2].fields.push_back( fieldno);
				}
			}
		}
		else if (stk.back().node->item.isOperator())
		{
			// We get to an operator, so we push it on the stack to process its children first (depth first):
			stk.push_back( stk.back().node->chld);
		}
		else
		{
			// We get to a leaf, so we define its query field to analyze:
			const QueryItem& item = stk.back().node->item;
			int fieldno = ++fieldnocnt;
			ctx->putField( fieldno, item.type, item.value);
			bool doAutoGroup = (0!=std::strchr( item.value.c_str(), ' ') || g_random.get( 0, 2) == 0);
			if (doAutoGroup)
			{
				std::vector<int> fieldnoList;
				fieldnoList.push_back( fieldno);
				ctx->groupElements( ImplicitSeqGroupId, fieldnoList, strus::QueryAnalyzerContextInterface::GroupAll, false);
			}
			// Go to the next element and add the query fieldno to the parents context:
			stk.back().node = stk.back().node->next;
			if (stk.size() > 1)
			{
				stk[ stk.size()-2].fields.push_back( fieldno);
			}
		}
	} while (!stk.empty());
	return opnodear;
}

static QueryTree buildAnalyzerResultTree( const strus::analyzer::QueryTermExpression& expr, const std::vector<const QueryTree*>& opnodear)
{
	std::vector<QueryTree> stk;

	std::vector<strus::analyzer::QueryTermExpression::Instruction>::const_iterator ii = expr.instructions().begin(), ie = expr.instructions().end();
	for (; ii != ie; ++ii)
	{
		switch (ii->opCode())
		{
			case strus::analyzer::QueryTermExpression::Instruction::Term:
			{
				const strus::analyzer::QueryTerm& term = expr.term( ii->idx());
				stk.push_back( QueryTree( QueryItem( term.type(), term.value(), term.len())));
				break;
			}
			case strus::analyzer::QueryTermExpression::Instruction::Operator:
			{
				std::size_t ni=0,ne = ii->nofOperands();
				if (ne > stk.size())
				{
					throw std::runtime_error("analyzer result is corrupt (not all arguments of node on the stack)");
				}
				QueryItem opitem;
				if (ii->idx() == ImplicitSeqGroupId)
				{
					opitem = QueryItem( "seq", ne-1, 0);
				}
				else
				{
					const QueryItem& op = opnodear.at( ii->idx()-1)->item;
					opitem = QueryItem( op.value, op.range, op.cardinality);
				}
				QueryTree opnode( opitem);
				for (; ni != ne; ++ni)
				{
					opnode.addChild( stk[ stk.size() - ne + ni]);
				}
				stk.erase( stk.end() - ne, stk.end());
				stk.push_back( opnode);
				break;
			}
			default:
				throw std::logic_error("analyzer result is corrupt (opcode)");
		}
	}
	if (stk.size() == 1)
	{
		return stk[ 0];
	}
	else
	{
		QueryTree rt( QueryItem( "", 0, 0));
		std::vector<QueryTree>::const_iterator si = stk.begin(), se = stk.end();
		for (; si != se; ++si)
		{
			rt.addChild( *si);
		}
		return rt;
	}
}

#ifdef STRUS_LOWLEVEL_DEBUG
static void printInstructions( std::ostream& out, const strus::analyzer::QueryTermExpression& expr, const std::vector<const QueryTree*>& opnodear)
{
	std::vector<strus::analyzer::QueryTermExpression::Instruction>::const_iterator ii = expr.instructions().begin(), ie = expr.instructions().end();
	for (; ii != ie; ++ii)
	{
		std::cerr << ii->opCodeName( ii->opCode());
		switch (ii->opCode())
		{
			case strus::analyzer::QueryTermExpression::Instruction::Term:
			{
				const strus::analyzer::QueryTerm& term = expr.term( ii->idx());
				std::cerr << " " << term.type() << " '" << term.value() << "' " << term.len();
				break;
			}
			case strus::analyzer::QueryTermExpression::Instruction::Operator:
			{
				if (ii->idx() == ImplicitSeqGroupId)
				{
					std::cerr << "seq " << ii->nofOperands() << std::endl;
				}
				else
				{
					const QueryTree* opnode = opnodear.at( ii->idx()-1);
					std::cerr << " " << opnode->item.value << " " << opnode->item.range << " " << opnode->item.cardinality;
				}
				break;
			}
			default:
				throw std::logic_error("analyzer result is corrupt (opcode)");
		}
		std::cerr << std::endl;
	}
}
#endif

static QueryTree analyzeQueryTree( const strus::QueryAnalyzerInterface* qana, const QueryTree& qt)
{
	strus::local_ptr<strus::QueryAnalyzerContextInterface> ctx( qana->createContext());
	if (!ctx.get()) throw std::runtime_error( g_errorhnd->fetchError());

	// Build the query tree:
	std::vector<const QueryTree*> opnodear = buildAnalyzerQueryTree( ctx.get(), qt);

	// Analyze the query tree:
	strus::analyzer::QueryTermExpression expr = ctx->analyze();

#ifdef STRUS_LOWLEVEL_DEBUG
	// Print the instructions produced by analysis:
	std::cerr << "Query analysis instructions:" << std::endl;
	printInstructions( std::cerr, expr, opnodear);
#endif
	// Build and return the result tree:
	return buildAnalyzerResultTree( expr, opnodear);
}

char const* skipSpaces( char const* si)
{
	while (*si && (unsigned char)*si <= 32) ++si;
	return si;
}

std::string parseToken( char const*& si)
{
	std::string rt;
	while (*si && (unsigned char)*si > 32)
	{
		rt.push_back( *si++);
	}
	return rt;
}

std::vector<std::string> split( char const* si)
{
	std::vector<std::string> res;
	for (si=skipSpaces(si); *si; si=skipSpaces(si))
	{
		res.push_back( parseToken( si));
	}
	return res;
}

static int countWords( char const* si)
{
	return split( si).size();
}

static QueryTree mapExpectedAnalyzedQueryTree( const QueryTree& qt, const char** desttypes)
{
	int didx = charArrayLength( desttypes);
	if (!didx) throw std::runtime_error("empty feature type definitions");

	if (qt.item.isOperator())
	{
		QueryTree res( QueryItem( qt.item.value, qt.item.range, qt.item.cardinality));
		QueryTree* nd = qt.chld;
		for (; nd; nd=nd->next)
		{
			res.addChild( mapExpectedAnalyzedQueryTree( *nd, desttypes));
		}
		return res;
	}
	else
	{
		int factor = fieldTypeValue( qt.item.type);
		std::string restype = desttypes[ factor % didx];
		char const* si = qt.item.value.c_str();
		int nofWords = countWords( si);
		if (nofWords == 0)
		{
			throw std::runtime_error("empty token in query");
		}
		else if (nofWords == 1)
		{
			std::string value = normalizeMultiplication( qt.item.value, factor);
			return QueryTree( QueryItem( restype, value, qt.item.len));
		}
		else
		{
			int range = nofWords;
			int cardinality = 0;
			QueryTree res( QueryItem( "seq", range, cardinality));

			std::vector<std::string> tokens = split( si);
			std::vector<std::string>::const_iterator ti = tokens.begin(), te = tokens.end();
	
			for (; ti != te; ++ti)
			{
				std::string value = normalizeMultiplication( *ti, factor);
				int len = 1;
				res.addChild( QueryItem( restype, value, len));
			}
			return res;
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
	else if (argc < 4)
	{
		std::cerr << "ERROR too few parameters" << std::endl;
		printUsage( argc, argv);
		return 1;
	}
	else if (argc > 4)
	{
		std::cerr << "ERROR too many parameters" << std::endl;
		printUsage( argc, argv);
		return 1;
	}
	try
	{
		std::string outputdir( argv[1]);
		std::string expectedfile = outputdir + strus::dirSeparator() + "EXP";
		std::string outputfile = outputdir + strus::dirSeparator() + "RES";

		int ec = strus::createDir( outputdir, false);
		if (ec) throw std::runtime_error( strus::string_format( "failed to create output directory '%s'", outputdir.c_str()));

		g_errorhnd = strus::createErrorBuffer_standard( 0, 2, NULL/*debug trace interface*/);
		if (!g_errorhnd)
		{
			throw std::runtime_error("failed to create error buffer object");
		}
		int nofTests = parseInt( argv[2]);
		int maxTreeSize = parseInt( argv[3]);
		int typemax = (int)(std::sqrt(maxTreeSize+1));
		int opmax = (int)(std::log(maxTreeSize+1));
		int valuemax = maxTreeSize+1;
		static const char* desttypes[] = { "even", "odd", 0 };

		strus::local_ptr<strus::TextProcessorInterface> textproc( createTextProcessor( g_errorhnd));
		if (!textproc.get()) throw std::runtime_error( g_errorhnd->fetchError());
		strus::local_ptr<strus::QueryAnalyzerInterface> analyzer( strus::createQueryAnalyzer( g_errorhnd));
		if (!analyzer.get()) throw std::runtime_error( g_errorhnd->fetchError());
		defineQueryAnalysis( analyzer.get(), textproc.get(), typemax, desttypes);

		int ti=1,te=nofTests+1;
		for (; ti<te; ++ti)
		{
#ifdef STRUS_LOWLEVEL_DEBUG
			std::cerr << "Test " << ti << std::endl;
#endif
			int childmax = g_random.get( 1, 10);
			int depth = 1 + (int)(std::log(maxTreeSize+1) / std::log(childmax+1));
#ifdef STRUS_LOWLEVEL_DEBUG
			std::cerr << "randomQueryTree( " << maxTreeSize << ", " << childmax << ", " << depth << ", " << opmax << ", " << typemax << ", " << valuemax << ");" << std::endl;			
#endif
			QueryTree querytree( randomQueryTree( maxTreeSize, childmax, depth, opmax, typemax, valuemax));

#ifdef STRUS_LOWLEVEL_DEBUG
			std::cerr << "tree dump before analysis:" << std::endl;
			querytree.print( std::cerr);
			std::cerr << std::endl;
#endif
			QueryTree resulttree = analyzeQueryTree( analyzer.get(), querytree);
			std::ostringstream resultstream;
			resulttree.print( resultstream);
			std::string outputstr = resultstream.str();
#ifdef STRUS_LOWLEVEL_DEBUG
			std::cerr << "tree dump after analysis:" << std::endl << outputstr << std::endl;
#endif
			QueryTree expecttree = mapExpectedAnalyzedQueryTree( querytree, desttypes);
			std::ostringstream expectstream;
			resulttree.print( expectstream);
			std::string expectedstr = expectstream.str();
#ifdef STRUS_LOWLEVEL_DEBUG
			std::cerr << "expected tree after analysis:" << std::endl << expectedstr;
#endif
			if (g_errorhnd->hasError())
			{
				throw std::runtime_error( g_errorhnd->fetchError());
			}
			if (outputstr != expectedstr)
			{
				ec = strus::writeFile( outputfile, outputstr);
				if (ec) throw std::runtime_error( strus::string_format( "failed to write output file '%s'", outputfile.c_str()));
				ec = strus::writeFile( expectedfile, expectedstr);
				if (ec) throw std::runtime_error( strus::string_format( "failed to write expected file '%s'", expectedfile.c_str()));
				std::cerr << "output written to " << outputfile << std::endl;
				std::cerr << "expected written to " << expectedfile << std::endl;
				throw std::runtime_error("output not as expected");
			}
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


