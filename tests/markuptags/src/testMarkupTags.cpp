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
#include "strus/base/utf8.hpp"
#include "strus/base/math.hpp"
#include "strus/base/pseudoRandom.hpp"
#include "private/textEncoder.hpp"
#include "textwolf/charset_utf8.hpp"
#include "tree.hpp"
#include <memory>
#include <string>
#include <vector>
#include <cstring>
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <cctype>
#include <algorithm>

static strus::ErrorBufferInterface* g_errorhnd = 0;
static strus::FileLocatorInterface* g_fileLocator = 0;
static bool g_verbose = false;
static int g_matches_found = 0;
static strus::PseudoRandom g_random;

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

	Type type() const								{return m_type;}
	const std::string& value() const						{return m_value;}
	const std::vector<strus::analyzer::DocumentAttribute>& attributes() const	{return m_attributes;}

	void addAttribute( const strus::analyzer::DocumentAttribute& attr)
	{
		std::vector<strus::analyzer::DocumentAttribute>::iterator ai = m_attributes.begin(), ae = m_attributes.end();
		for (; ai != ae; ++ai)
		{
			if (attr.name() == ai->name())
			{
				*ai = attr;
				break;
			}
		}
		if (ai == ae)
		{
			m_attributes.push_back( attr);
		}
	}

private:
	Type m_type;
	std::string m_value;
	std::vector<strus::analyzer::DocumentAttribute> m_attributes;
};

typedef strus::test::TreeNode<TestDocumentItem> TestDocumentTree;

std::ostream& operator << ( std::ostream& os, const TestDocumentItem& item)
{
	switch (item.type())
	{
		case TestDocumentItem::Content:
			os << item.value();
			break;
		break;
		case TestDocumentItem::Tag:
		{
			os << "<" << item.value();
			std::vector<strus::analyzer::DocumentAttribute>::const_iterator ai = item.attributes().begin(), ae = item.attributes().end();
			for (; ai != ae; ++ai)
			{
				os << " " << ai->name() << "=\"" << ai->value() << "\"";
			}
			os << ">";
			break;
		}
	}
	return os;
}

void printTreeXML( std::ostream& out, const TestDocumentTree& tree)
{
	TestDocumentTree const* np = &tree;
	do
	{
		switch (np->item.type())
		{
			case TestDocumentItem::Content:
				out << np->item.value();
				break;
			break;
			case TestDocumentItem::Tag:
			{
				out << "<" << np->item.value();
				std::vector<strus::analyzer::DocumentAttribute>::const_iterator ai = np->item.attributes().begin(), ae = np->item.attributes().end();
				for (; ai != ae; ++ai)
				{
					out << " " << ai->name() << "=\"" << ai->value() << "\"";
				}
				if (np->chld)
				{
					out << ">\n";
					printTreeXML( out, *np->chld);
					out << "</" << np->item.value() << ">";
				}
				else
				{
					out << "/>";
				}
				break;
			}
		}
		np = np->next;
	} while (np);
}


enum {NofTags=30};
enum {NofAttributes=20};
enum {MaxAttributeLength=100};
enum {MaxValueLength=1000};

static std::string createRandomContentValue( int maxlength)
{
	std::string rt;
	textwolf::charset::UTF8 out;

	int v1 = g_random.get( 0, maxlength);
	int v2 = g_random.get( 0, v1+1);
	int length = g_random.get( 0, v2+1);
	int li=0, le=length;
	for (; li != le; ++li)
	{
		textwolf::UChar chr = 0;
		if (g_random.get( 0, 8) == 0 && !rt.empty() && rt[rt.size()] != ' ')
		{
			chr = 32;
		}
		else if (g_random.get( 0, 10) > 0)
		{
			static const char alphabet[] = "aaaabcdeeeeeefghiijklmnoopqrssstuvwxyz0123456789";
			char ac = alphabet[ g_random.get( 0, std::strlen( alphabet))];
			if (g_random.get( 0, 2) > 0)
			{
				ac = std::toupper( ac);
			}
			chr = ac;
		}
		else
		{
			static char codepagedistr[] = {1,1,1,1,1,1,2,2,2,2,3,3,3,4,4,4,5,5,5,6,6,7,7,8,8,9,10,11,12,13,0};
			int maxchr = 128 << codepagedistr[ g_random.get( 0, std::strlen(codepagedistr))];
			chr = g_random.get( 127, maxchr);
		}
		out.print( chr, rt);
	}
	return rt;
}

static std::string createRandomAttributeValue( int maxlength)
{
	std::string rt;
	textwolf::charset::UTF8 out;

	int v1 = g_random.get( 0, maxlength);
	int v2 = g_random.get( 0, v1+1);
	int length = g_random.get( 0, v2+1);
	int li=0, le=length;
	for (; li != le; ++li)
	{
		textwolf::UChar chr = 0;
		if (g_random.get( 0, 8) == 0 && !rt.empty() && rt[rt.size()] != ' ')
		{
			chr = 32;
		}
		else
		{
			static const char alphabet[] = "aaaabcdeeeeeefghiijklmnoopqrssstuvwxyz0123456789";
			char ac = alphabet[ g_random.get( 0, std::strlen( alphabet))];
			if (g_random.get( 0, 2) > 0)
			{
				ac = std::toupper( ac);
			}
			chr = ac;
		}
		out.print( chr, rt);
	}
	return rt;
}

static bool attributeDefined( const std::vector<strus::analyzer::DocumentAttribute>& attributes, const std::string& name)
{
	std::vector<strus::analyzer::DocumentAttribute>::const_iterator ai = attributes.begin(), ae = attributes.end();
	for (; ai != ae && ai->name() != name; ++ai){}
	if (ai != ae)
	{
		return true; //... again, no duplicates
	}
	return false;
}

static TestDocumentItem createRandomTag()
{
	enum {AttributesDistrSize=14};
	static const int nofAttributesDistr[AttributesDistrSize] = {0,0,0,0,1,1,1,1,2,2,2,3,3,4};
	std::vector<strus::analyzer::DocumentAttribute> attributes;
	int nofAttributes = nofAttributesDistr[ g_random.get( 0,AttributesDistrSize)];
	int ai=0,ae=nofAttributes;
	for (; ai != ae; ++ai)
	{
		int attridx = g_random.get( 0, g_random.get( 0, g_random.get( 0, NofAttributes)+1)+1);
		std::string attrname = strus::string_format( "A%d", attridx);
		if (attributeDefined( attributes, attrname))
		{
			if (nofAttributes == (int)attributes.size()) break;
			--ai; continue; //... again, no duplicates
		}
		std::string attrvalue = createRandomAttributeValue( MaxAttributeLength);
		attributes.push_back( strus::analyzer::DocumentAttribute( attrname, attrvalue));
	}
	int tagidx = g_random.get( 0, g_random.get( 0, g_random.get( 0, NofTags)+1)+1);
	std::string tagname = strus::string_format( "T%d", tagidx);
	return TestDocumentItem( TestDocumentItem::Tag, tagname, attributes);
}

static TestDocumentItem createRandomTestDocumentItem( int potentialTag)
{
	if (!potentialTag || g_random.get( 0, g_random.get( 0, potentialTag)+1) == 0)
	{
		std::string value = createRandomContentValue( MaxValueLength);
		return TestDocumentItem( TestDocumentItem::Content, value);
	}
	else
	{
		return createRandomTag();
	}
}

static TestDocumentTree createRandomTestDocumentTree_( int complexity, int& nofNodes)
{
	TestDocumentItem node = createRandomTestDocumentItem( complexity);
	TestDocumentTree rt( node);
	if (node.type() == TestDocumentItem::Tag && complexity)
	{
		int nofChildren = g_random.get( 0, (int)strus::Math::sqrt( (double)complexity));
		int ci=0,ce=nofChildren;
		for (; ci != ce && nofNodes < complexity * complexity; ++ci)
		{
			TestDocumentTree chld = createRandomTestDocumentTree_( g_random.get( 0, g_random.get( 0, complexity)+1), nofNodes);
			++nofNodes;
			rt.addChild( chld);
		}
	}
	return rt;
}

static TestDocumentTree createRandomTestDocumentTree( int complexity)
{
	int nofNodes = 0;
	return createRandomTestDocumentTree_( complexity, nofNodes);
}

class TestAttributeMarkup
	:public strus::TagAttributeMarkupInterface
{
public:
	TestAttributeMarkup( const char* attrname_)
		:m_attrname( attrname_)
	{}
	virtual ~TestAttributeMarkup(){}

	virtual strus::analyzer::DocumentAttribute synthesizeAttribute( const std::string& tagname, const std::vector<strus::analyzer::DocumentAttribute>& attributes) const
	{
		std::string val;
		val.append( tagname);
		std::vector<strus::analyzer::DocumentAttribute>::const_iterator ai = attributes.begin(), ae = attributes.end();
		for (; ai != ae; ++ai)
		{
			val.push_back( ' ');
			val.append( ai->name());
			val.push_back( ':');
			val.append( ai->value());
		}
		return strus::analyzer::DocumentAttribute( m_attrname, val + strus::string_format( " N:%d", (int)attributes.size()));
	}
private:
	std::string m_attrname;
};

static bool isAlpha( char ch)
{
	return ((ch|32) >= 'a' && (ch|32) <= 'z') || ch == '_';
}

static bool isDigit( char ch)
{
	return (ch >= '0' && ch <= '9');
}

static bool isAlphaNum( char ch)
{
	return isAlpha(ch) || isDigit(ch);
}

struct MatchExpression
{
	MatchExpression( const std::string& selectexpr)
	{
		root = !(selectexpr.size() > 2 && selectexpr[0] == '/' && selectexpr[1] == '/');
		char const* si = selectexpr.c_str();
		if (*si == '/') ++si;
		if (*si == '/') ++si;
		if (!isAlpha(*si)) throw std::runtime_error( "unsupported expression for test");
		for (; isAlphaNum(*si); ++si)
		{
			tagname.push_back( *si);
		}
		if (*si == '@')
		{
			for (++si; isAlphaNum(*si); ++si)
			{
				attrname.push_back( *si);
			}
		}
		if (tagname.empty() && attrname.empty()) throw std::runtime_error( "empty expression for test");
		if (*si) throw std::runtime_error( "unsupported expression for test");
	}

	bool matches( const TestDocumentItem& item, int depth) const
	{
		switch (item.type())
		{
			case TestDocumentItem::Content:
				return false;
			break;
			case TestDocumentItem::Tag:
			{
				if (root && depth) return false;
				if (!tagname.empty() && item.value() != tagname) return false;

				if (attrname.empty()) return true;
				std::vector<strus::analyzer::DocumentAttribute>::const_iterator ai = item.attributes().begin(), ae = item.attributes().end();
				for (; ai != ae && ai->name() != attrname; ++ai){}
				if (ai == ae) return false;
				return true;
			}
			break;
		}
		return false;
	}
	bool root;
	std::string tagname;
	std::string attrname;
};

static void doTestTagMarkup( TestDocumentTree* np, const MatchExpression& selectexpr, const TestAttributeMarkup& markup, int& matchcnt, int depth=0)
{
	do
	{
		if (selectexpr.matches( np->item, depth))
		{
			++matchcnt;
			++g_matches_found;
			if (g_verbose) std::cerr << "found match tag='" << selectexpr.tagname << "', attr='" << selectexpr.attrname << "' for [" << np->item << "]" << std::endl;
			
			strus::analyzer::DocumentAttribute attr = markup.synthesizeAttribute( selectexpr.tagname, np->item.attributes());
			np->item.addAttribute( attr);
		}
		if (np->chld)
		{
			doTestTagMarkup( np->chld, selectexpr, markup, matchcnt, depth+1);
		}
		np = np->next;
	} while (np);
}

static void runSimpleTest( const strus::TextProcessorInterface* textproc, const TestDocumentTree& tree, const char* selectexpr, const char* attrname)
{
	int notMatches = 0;
	std::ostringstream testtreebuf;
	printTreeXML( testtreebuf, tree);
	std::string xmlheader = strus::string_format( "<?xml version=\"1.0\" encoding=\"%s\" standalone=\"yes\"?>\n", "UTF-8");
	std::string inputstr = xmlheader + testtreebuf.str() + "\n";

	MatchExpression matchExpression( selectexpr); 
	TestAttributeMarkup markupTag( attrname);

	std::vector<strus::DocumentTagMarkupDef> markups;
	markups.push_back( strus::DocumentTagMarkupDef( new TestAttributeMarkup( attrname), selectexpr));

	TestDocumentTree expectedtree( tree);
	doTestTagMarkup( &expectedtree, matchExpression, markupTag, notMatches);

	std::ostringstream expectedtreebuf;
	printTreeXML( expectedtreebuf, expectedtree);
	std::string expectedstr = xmlheader + expectedtreebuf.str() + "\n";

	strus::analyzer::DocumentClass documentClass( "application/xml", "UTF-8");
	std::string resultstr = strus::markupDocumentTags( documentClass, inputstr, markups, textproc, g_errorhnd);

	if (expectedstr != resultstr || g_verbose)
	{
		std::cerr << "TEST " << selectexpr << " " << attrname << std::endl;
		std::cerr << "INPUT:" << std::endl << inputstr << std::endl;
		std::cerr << "RESULT:" << std::endl << resultstr << std::endl;
		std::cerr << "EXPECTED:" << std::endl << expectedstr << std::endl;
	}
	if (expectedstr != resultstr)
	{
		throw std::runtime_error( strus::string_format( "Simple test %s %s failed", selectexpr, attrname));
	}
}

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
		else if (std::strcmp( argv[argi], "--") == 0)
		{
			++argi;
			break;
		}
		else if (argv[argi][0] == '-')
		{
			std::cerr << "unknown option " << argv[argi] << std::endl;
			printUsage( argc, argv);
			return 1;
		}
		else
		{
			break;
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
		++argi;
	}
	if (argi < argc)
	{
		complexity = atoi( argv[argi]);
		if (complexity <= 0)
		{
			std::cerr << "bad argument for complexity of tests: " << argv[argi] << std::endl;
			return 1;
		}
		++argi;
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
		int matchcnt[16] = {0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0};
		// Run simple tests:
		{
			TestDocumentItem root( TestDocumentItem::Tag, "T1");
			TestDocumentItem content( TestDocumentItem::Content, "Here come the jets");
			TestDocumentTree tree( root);
			tree.addChild( content);
			runSimpleTest( textproc.get(), tree, "/T1", "RootTag");
		}
		{
			TestDocumentItem root( TestDocumentItem::Tag, "T1");
			TestDocumentItem chld( TestDocumentItem::Tag, "T2");
			TestDocumentItem content( TestDocumentItem::Content, "Here come the jets");
			TestDocumentTree subtree( chld);
			subtree.addChild( content);
			TestDocumentTree tree( root);
			tree.addChild( subtree);
			runSimpleTest( textproc.get(), tree, "//T2", "AnyTag");
		}
		{
			std::vector<strus::analyzer::DocumentAttribute> attr;
			attr.push_back( strus::analyzer::DocumentAttribute( "A1", "a1"));
			attr.push_back( strus::analyzer::DocumentAttribute( "A2", "a2"));
			TestDocumentItem root( TestDocumentItem::Tag, "T1", attr);
			TestDocumentItem chld( TestDocumentItem::Tag, "T3");
			TestDocumentItem content( TestDocumentItem::Content, "Here come the jets");
			TestDocumentTree subtree( chld);
			subtree.addChild( content);
			TestDocumentTree tree( root);
			tree.addChild( subtree);
			runSimpleTest( textproc.get(), tree, "/T1@A1", "RootTagAttr");
		}
		{
			TestDocumentItem root( TestDocumentItem::Tag, "T1");
			std::vector<strus::analyzer::DocumentAttribute> attr;
			attr.push_back( strus::analyzer::DocumentAttribute( "A1", "a1"));
			attr.push_back( strus::analyzer::DocumentAttribute( "A2", "a2"));
			TestDocumentItem chld( TestDocumentItem::Tag, "T3", attr);
			TestDocumentItem content( TestDocumentItem::Content, "Here come the jets");
			TestDocumentTree subtree( chld);
			subtree.addChild( content);
			TestDocumentTree tree( root);
			tree.addChild( subtree);
			runSimpleTest( textproc.get(), tree, "//T3@A1", "AnyTagAttr");
		}
		{
			TestDocumentItem root( TestDocumentItem::Tag, "T1");
			std::vector<strus::analyzer::DocumentAttribute> attr;
			attr.push_back( strus::analyzer::DocumentAttribute( "A1", "a1"));
			attr.push_back( strus::analyzer::DocumentAttribute( "A2", "a2"));
			TestDocumentItem chld( TestDocumentItem::Tag, "T3", attr);
			TestDocumentItem content( TestDocumentItem::Content, "Here come the jets");
			TestDocumentTree subtree( chld);
			subtree.addChild( content);
			TestDocumentTree tree( root);
			tree.addChild( subtree);
			runSimpleTest( textproc.get(), tree, "//T3@A1", "A1");
		}
		// Run random document tests:
		int ti=1,te=nofTests+1;
		{
			for (; ti != te; ++ti)
			{
				// Define markups:
				MatchExpression selectRootTag( "/T1"); 
				MatchExpression selectAnyTag( "//T2");
				MatchExpression selectRootTagAttr( "/T3@A1");
				MatchExpression selectAnyTagAttr( "//T4@A1");
				MatchExpression selectAnyTagReplace( "//T5@A1");
				TestAttributeMarkup markupRootTag( "RootTag");
				TestAttributeMarkup markupAnyTag( "AnyTag");
				TestAttributeMarkup markupRootTagAttr( "RootTagAttr");
				TestAttributeMarkup markupAnyTagAttr( "AnyTagAttr");
				TestAttributeMarkup markupAnyTagAttrReplace( "A1");

				std::vector<strus::DocumentTagMarkupDef> markups;
				markups.push_back( strus::DocumentTagMarkupDef( new TestAttributeMarkup( "RootTag"), "/T1"));
				markups.push_back( strus::DocumentTagMarkupDef( new TestAttributeMarkup( "AnyTag"), "//T2"));
				markups.push_back( strus::DocumentTagMarkupDef( new TestAttributeMarkup( "RootTagAttr"), "/T3@A1"));
				markups.push_back( strus::DocumentTagMarkupDef( new TestAttributeMarkup( "AnyTagAttr"), "//T4@A1"));
				markups.push_back( strus::DocumentTagMarkupDef( new TestAttributeMarkup( "A1"), "//T5@A1"));

				// Create test document tree:
				TestDocumentTree testtree = createRandomTestDocumentTree( complexity);
				if (testtree.next || testtree.item.type() == TestDocumentItem::Content)
				{
					TestDocumentTree root( TestDocumentItem( TestDocumentItem::Tag, g_random.get(0,2) == 1 ? "T1":"T3"));
					root.addChild( testtree);
					testtree = root;
				}
				std::ostringstream testtreebuf;
				printTreeXML( testtreebuf, testtree);
				std::string inputstr = testtreebuf.str();

				// Build expected result:
				TestDocumentTree resulttree( testtree);
				doTestTagMarkup( &resulttree, selectRootTag, markupRootTag, matchcnt[0]);
				doTestTagMarkup( &resulttree, selectAnyTag, markupAnyTag, matchcnt[1]);
				doTestTagMarkup( &resulttree, selectRootTagAttr, markupRootTagAttr, matchcnt[2]);
				doTestTagMarkup( &resulttree, selectAnyTagAttr, markupAnyTagAttr, matchcnt[3]);
				doTestTagMarkup( &resulttree, selectAnyTagReplace, markupAnyTagAttrReplace, matchcnt[4]);
				std::ostringstream expectedtreebuf;
				printTreeXML( expectedtreebuf, resulttree);
				std::string expectedstr = expectedtreebuf.str();

				// Process document:
				std::string resultstr_xml;
				enum {NofCharsets=5};
				static const char* charsets[ NofCharsets] = {"UTF-8","UTF-16BE","UTF-16LE","UTF-32BE","UTF-32LE"};
				int charsetidx = g_random.get( 0, NofCharsets);
				strus::local_ptr<strus::utils::TextEncoderBase> encoder( strus::utils::createTextEncoder( charsets[ charsetidx]));
				strus::local_ptr<strus::utils::TextEncoderBase> decoder( strus::utils::createTextDecoder( charsets[ charsetidx]));
				if (!encoder.get() || !decoder.get()) throw std::runtime_error( strus::string_format( "failed to create text encoder/decoder for the character set encoding %s: %s", charsets[ charsetidx], g_errorhnd->fetchError()));
				std::string inputstr_xml_utf8 = strus::string_format( "<?xml version=\"1.0\" encoding=\"%s\" standalone=\"yes\"?>\n", charsets[ charsetidx])
								+ inputstr + "\n";
				std::string inputstr_xml = encoder->convert( inputstr_xml_utf8.c_str(), inputstr_xml_utf8.size(), true);
				bool doDetectDocumentClass = g_random.get( 0, 10) == 1;
				if (doDetectDocumentClass)
				{
					resultstr_xml = strus::markupDocumentTags( strus::analyzer::DocumentClass(), inputstr_xml, markups, textproc.get(), g_errorhnd);
				}
				else
				{
					strus::analyzer::DocumentClass documentClass( "application/xml", charsets[ charsetidx]);
					resultstr_xml = strus::markupDocumentTags( documentClass, inputstr_xml, markups, textproc.get(), g_errorhnd);
				}
				if (resultstr_xml.empty() && g_errorhnd->hasError())
				{
					ec = strus::writeFile( "INP", inputstr_xml);
					if (ec) throw std::runtime_error( strus::string_format( "error writing file %s: %s", "INP", ::strerror(ec)));
					throw std::runtime_error( "error in markup document");
				}
				std::string expectedstr_xml_utf8 = strus::string_format( "<?xml version=\"1.0\" encoding=\"%s\" standalone=\"yes\"?>\n", charsets[ charsetidx])
								+ expectedstr + "\n";
				std::string expectedstr_xml = encoder->convert( expectedstr_xml_utf8.c_str(), expectedstr_xml_utf8.size(), true);

				// Compare result with expected:
				if (resultstr_xml != expectedstr_xml)
				{
					ec = strus::writeFile( "INP", inputstr_xml);
					if (ec) throw std::runtime_error( strus::string_format( "error writing file %s: %s", "INP", ::strerror(ec)));
					ec = strus::writeFile( "RES", resultstr_xml);
					if (ec) throw std::runtime_error( strus::string_format( "error writing file %s: %s", "RES", ::strerror(ec)));
					ec = strus::writeFile( "EXP", expectedstr_xml);
					if (ec) throw std::runtime_error( strus::string_format( "error writing file %s: %s", "EXP", ::strerror(ec)));
					throw std::runtime_error( strus::string_format( "output of test %d not as expected", ti));
				}
			}
		}
		std::cerr << strus::string_format( "found %s %d times", "/T1", matchcnt[0]) << std::endl; 
		std::cerr << strus::string_format( "found %s %d times", "//T2", matchcnt[1]) << std::endl; 
		std::cerr << strus::string_format( "found %s %d times", "/T3@A1", matchcnt[2]) << std::endl; 
		std::cerr << strus::string_format( "found %s %d times", "//T4@A1", matchcnt[3]) << std::endl; 
		std::cerr << strus::string_format( "found %s %d times", "//T5@A1 (replace A1)", matchcnt[4]) << std::endl; 

		std::cerr << strus::string_format( "%d matches verified in %d random documents", g_matches_found, nofTests) << std::endl;
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


