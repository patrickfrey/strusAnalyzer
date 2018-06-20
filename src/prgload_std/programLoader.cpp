/*
 * Copyright (c) 2018 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Standard program loader class of the analyzer (load program in a domain specific language)
#include "programLoader.hpp"
#include "patternMatchProgramParser.hpp"
#include "strus/lib/pattern_serialize.hpp"
#include "strus/errorBufferInterface.hpp"
#include "strus/queryAnalyzerInstanceInterface.hpp"
#include "strus/documentAnalyzerInstanceInterface.hpp"
#include "strus/documentAnalyzerMapInterface.hpp"
#include "strus/textProcessorInterface.hpp"
#include "strus/base/programLexer.hpp"
#include "strus/base/fileio.hpp"
#include "strus/base/string_format.hpp"
#include "strus/base/string_conv.hpp"
#include "strus/base/numstring.hpp"
#include "strus/base/local_ptr.hpp"
#include "strus/reference.hpp"
#include "strus/normalizerFunctionInterface.hpp"
#include "strus/normalizerFunctionInstanceInterface.hpp"
#include "strus/tokenizerFunctionInterface.hpp"
#include "strus/tokenizerFunctionInstanceInterface.hpp"
#include "strus/aggregatorFunctionInterface.hpp"
#include "strus/aggregatorFunctionInstanceInterface.hpp"
#include "strus/patternTermFeederInterface.hpp"
#include "strus/patternLexerInterface.hpp"
#include "strus/patternLexerInstanceInterface.hpp"
#include "strus/patternMatcherInterface.hpp"
#include "strus/patternMatcherInstanceInterface.hpp"
#include "private/internationalization.hpp"
#include "private/errorUtils.hpp"
#include <vector>
#include <string>

using namespace strus;

enum Tokens {
	TokIdentifier,
	TokFloat,
	TokInteger,
	TokOpenOvalBracket,
	TokCloseOvalBracket,
	TokOpenCurlyBracket,
	TokCloseCurlyBracket,
	TokOpenSquareBracket,
	TokCloseSquareBracket,
	TokAssign,
	TokExp,
	TokComma,
	TokColon,
	TokSemiColon,
	TokLeftArrow,
	TokPath
};
static const char* g_tokens[] = {
	"[a-zA-Z_][a-zA-Z0-9_]*",
	"[+-]*[0-9][0-9_]*[.][0-9]*",
	"[+-]*[0-9][0-9_]*",
	"\\(",
	"\\)",
	"\\{",
	"\\}",
	"\\[",
	"\\]",
	"\\=",
	"\\^",
	",",
	":",
	";",
	"<-",
	"[/][^;,{} ]*",
	NULL
};
static const char* g_token_names[] = {
	"identifier",
	"flating point number",
	"integer",
	"open oval bracket '('",
	"close oval bracket ')'",
	"open curly bracket '{'",
	"close curly bracket '}'",
	"open square bracket '['",
	"close square bracket ']'",
	"assign '='",
	"exp '^'",
	"comma ','",
	"colon ':'",
	"semicolon ';'",
	"left arrow '<-'",
	"path",
	NULL
};
static const char* g_errtokens[] = {
	"[0-9][0-9]*[a-zA-Z_]",
	NULL
};
static const char* g_eolncomment = "#";

static const char* tokenName( const ProgramLexem& cur)
{
	switch (cur.type())
	{
		case ProgramLexem::Eof:		return "EOF";
		case ProgramLexem::SQString:	return "string";
		case ProgramLexem::DQString:	return "string";
		case ProgramLexem::Error:	return "bad lexem";
		case ProgramLexem::Token:	return g_token_names[ cur.id()];
	}
	return "?";
}

enum DocumentFeatureClass
{
	FeatSearchIndexTerm,
	FeatForwardIndexTerm,
	FeatMetaData,
	FeatAttribute,
	FeatPatternLexem
};

enum QueryElementClass
{
	QueryElementTerm,
	QueryElementLexem
};

static bool isEqual( const std::string& id, const char* idstr)
{
	char const* si = id.c_str();
	char const* di = idstr;
	for (; *si && *di && ((*si|32) == (*di|32)); ++si,++di){}
	return !*si && !*di;
}

static bool getDocumentFeatureClassFromName( DocumentFeatureClass& result, const std::string& name)
{
	if (isEqual( name, "SearchIndex"))
	{
		result = FeatSearchIndexTerm;
	}
	else if (isEqual( name, "ForwardIndex"))
	{
		result = FeatForwardIndexTerm;
	}
	else if (isEqual( name, "MetaData"))
	{
		result = FeatMetaData;
	}
	else if (isEqual( name, "Attribute"))
	{
		result = FeatAttribute;
	}
	else if (isEqual( name, "PatternLexem"))
	{
		result = FeatPatternLexem;
	}
	else
	{
		return false;
	}
	return true;
}

static bool getQueryElementClassFromName( QueryElementClass& result, const std::string& name)
{
	if (isEqual( name, "Element"))
	{
		result = QueryElementTerm;
	}
	else if (isEqual( name, "PatternLexem"))
	{
		result = QueryElementLexem;
	}
	else
	{
		return false;
	}
	return true;
}

static std::vector<std::string> parseArgumentList( ProgramLexer& lexer)
{
	std::vector<std::string> rt;
	for (;;)
	{
		ProgramLexem cur = lexer.current();
		if (cur.isToken( TokIdentifier) || cur.isToken( TokFloat) || cur.isToken( TokInteger))
		{
			rt.push_back( cur.value());
		}
		else if (cur.isString())
		{
			rt.push_back( cur.value());
		}
		else
		{
			throw strus::runtime_error( _TXT("unexpected token %s in argument list"), tokenName(cur));
		}
		cur = lexer.next();
		if (cur.isToken( TokComma))
		{
			cur = lexer.next();
			continue;
		}
		break;
	}
	return rt;
}

static void parseFunctionDef( ProgramLexer& lexer, const char* functype, std::string& name, std::vector<std::string>& arg)
{
	arg.clear();
	ProgramLexem cur = lexer.current();
	if (cur.isToken( TokIdentifier))
	{
		name = cur.value();
		cur = lexer.next();
		if (cur.isToken( TokOpenOvalBracket))
		{
			cur = lexer.next();
			if (cur.isToken( TokCloseOvalBracket))
			{
				lexer.next();
			}
			else
			{
				arg = parseArgumentList( lexer);
			}
			if (!lexer.current().isToken( TokCloseOvalBracket))
			{
				throw strus::runtime_error( _TXT("comma ',' as argument separator or close oval bracket ')' instead of %s expected at end of %s argument list"), tokenName( lexer.current()), functype);
			}
			lexer.next();
		}
	}
	else
	{
		throw strus::runtime_error( _TXT("identifier expected instead of %s at start of %s definition"), tokenName( lexer.current()), functype);
	}
}

/// \brief Description of a function (tokenizer/normalizer)
class FunctionConfig
{
public:
	FunctionConfig( const std::string& name_, const std::vector<std::string>& args_)
		:m_name(name_),m_args(args_){}
	FunctionConfig( const FunctionConfig& o)
		:m_name(o.m_name),m_args(o.m_args){}

	/// \brief Get the name of the tokenizer
	const std::string& name() const			{return m_name;}
	/// \brief Get the arguments of the tokenizer
	const std::vector<std::string>& args() const	{return m_args;}

private:
	std::string m_name;
	std::vector<std::string> m_args;
};

static std::vector<FunctionConfig> parseNormalizerConfig( ProgramLexer& lexer)
{
	std::vector<FunctionConfig> rt;
	for(;;)
	{
		std::string name;
		std::vector<std::string> arg;
		parseFunctionDef( lexer, "normalizer", name, arg);
		rt.insert( rt.begin(), FunctionConfig( name, arg));

		if (!lexer.current().isToken( TokColon)) break;
		lexer.next();
	}
	return rt;
}

static FunctionConfig parseTokenizerConfig( ProgramLexer& lexer)
{
	std::string name;
	std::vector<std::string> arg;
	parseFunctionDef( lexer, "tokenizer", name, arg);
	return FunctionConfig( name, arg);
}

static FunctionConfig parseAggregatorFunctionConfig( ProgramLexer& lexer)
{
	std::string name;
	std::vector<std::string> arg;
	parseFunctionDef( lexer, "aggregator", name, arg);
	return FunctionConfig( name, arg);
}

static analyzer::FeatureOptions parseFeatureOptions( ProgramLexer& lexer)
{
	analyzer::FeatureOptions rt;

	ProgramLexem cur = lexer.current();
	if (cur.isToken( TokOpenCurlyBracket))
	{
		do
		{
			cur = lexer.next();
			if (cur.isToken( TokIdentifier))
			{
				std::string optname = cur.value();
				std::string optval;

				cur = lexer.next();
				if (!cur.isToken( TokAssign))
				{
					throw strus::runtime_error( _TXT("assign '=' instead of %s expected after open curly brackets '{' and option identifier"), tokenName( lexer.current()));
				}
				cur = lexer.next();
				if (cur.isString() || cur.isToken( TokIdentifier))
				{
					optval = cur.value();
				}
				else
				{
					throw strus::runtime_error( _TXT("identifier or string expected instead of %s as option value"), tokenName( lexer.current()));
				}
				if (strus::caseInsensitiveEquals( optname, "position"))
				{
					if (strus::caseInsensitiveEquals( optval, "succ"))
					{
						rt.definePositionBind( analyzer::BindSuccessor);
					}
					else if (strus::caseInsensitiveEquals( optval, "pred"))
					{
						rt.definePositionBind( analyzer::BindPredecessor);
					}
					else if (strus::caseInsensitiveEquals( optval, "unique"))
					{
						rt.definePositionBind( analyzer::BindUnique);
					}
					else
					{
						throw strus::runtime_error( _TXT("'pred' or 'succ' expected instead of %s as 'position' option value"), optval.c_str());
					}
				}
				else
				{
					throw strus::runtime_error( _TXT("unknown option '%s'"), optname.c_str());
				}
				cur = lexer.next();
			}
			else
			{
				throw strus::runtime_error( _TXT("expected identifier instead of %s as token name"), tokenName( lexer.current()));
			}
		}
		while (cur.isToken( TokComma));

		if (!cur.isToken( TokCloseCurlyBracket))
		{
			throw strus::runtime_error( _TXT("close curly bracket '}' expected instead of %s at end of option list"), tokenName( lexer.current()));
		}
		lexer.next();
	}
	return rt;
}

static std::string parseSelectorExpression( ProgramLexer& lexer)
{
	ProgramLexem cur = lexer.current();
	if (cur.isString() || cur.isToken( TokPath) || cur.isToken( TokIdentifier))
	{
		std::string rt = cur.value();
		lexer.next();
		return rt;
	}
	else
	{
		throw strus::runtime_error( _TXT("expected string or path instead of %s as selector expression"), tokenName( lexer.current()));
	}
}

static bool isFileNameChar( char ch)
{
	if ((ch|32) >= 'a' && (ch|32) <= 'z') return true;
	if (ch >= '0' && ch <= '9') return true;
	if (ch == '.') return true;
	if (ch == '_') return true;
	if (ch == '-') return true;
	if (ch == '/') return true;
	if (ch == '\\') return true;
	return false;
}

static std::string parseFileName( ProgramLexer& lexer)
{
	ProgramLexem cur = lexer.current();
	if (cur.isString() || cur.isToken( TokPath) || cur.isToken( TokIdentifier))
	{
		const char* si = lexer.currentpos();
		std::string rt;
		for (; *si && isFileNameChar(*si) ; ++si)
		{
			rt.push_back( *si);
		}
		if (*si && (unsigned char)*si > 32)
		{
			throw strus::runtime_error(_TXT("illegal character in filename: ASCII %d"), (int)(unsigned int)(unsigned char)*si);
		}
		lexer.skipto( si);
		return rt;
	}
	else
	{
		throw strus::runtime_error( _TXT("expected string or path instead of %s as file name"), tokenName( lexer.current()));
	}
}

struct FeatureDef
{
	strus::local_ptr<TokenizerFunctionInstanceInterface> tokenizer;
	std::vector<Reference<NormalizerFunctionInstanceInterface> > normalizer_ref;
	std::vector<NormalizerFunctionInstanceInterface*> normalizer;

	~FeatureDef(){}

	void parseNormalizer( ProgramLexer& lexer, const TextProcessorInterface* textproc)
	{
		std::vector<FunctionConfig> normalizercfg = parseNormalizerConfig( lexer);
		std::vector<FunctionConfig>::const_iterator ni = normalizercfg.begin(), ne = normalizercfg.end();
		for (; ni != ne; ++ni)
		{
			const NormalizerFunctionInterface* nm = textproc->getNormalizer( ni->name());
			if (!nm) throw strus::runtime_error(_TXT( "normalizer function '%s' not found"), ni->name().c_str());
	
			Reference<NormalizerFunctionInstanceInterface> nmi( nm->createInstance( ni->args(), textproc));
			if (!nmi.get()) throw strus::runtime_error(_TXT( "failed to create instance of normalizer function '%s'"), ni->name().c_str());
	
			normalizer_ref.push_back( nmi);
			normalizer.push_back( nmi.get());
		}
	}

	void parseTokenizer( ProgramLexer& lexer, const TextProcessorInterface* textproc)
	{
		FunctionConfig tokenizercfg = parseTokenizerConfig( lexer);
		const TokenizerFunctionInterface* tk = textproc->getTokenizer( tokenizercfg.name());
		if (!tk) throw strus::runtime_error(_TXT( "tokenizer function '%s' not found"), tokenizercfg.name().c_str());
	
		tokenizer.reset( tk->createInstance( tokenizercfg.args(), textproc));
		if (!tokenizer.get()) throw strus::runtime_error(_TXT( "failed to create instance of tokenizer function '%s'"), tokenizercfg.name().c_str());
	}

	void release()
	{
		std::vector<Reference<NormalizerFunctionInstanceInterface> >::iterator
			ri = normalizer_ref.begin(), re = normalizer_ref.end();
		for (; ri != re; ++ri)
		{
			(void)ri->release();
		}
		(void)tokenizer.release();
	}
};

static void parseDocumentPatternFeatureDef(
	ProgramLexer& lexer,
	DocumentAnalyzerInstanceInterface& analyzer,
	const TextProcessorInterface* textproc,
	const std::string& featureName,
	int priority,
	DocumentFeatureClass featureClass)
{
	FeatureDef featuredef;

	// [1] Parse pattern item name:
	if (!lexer.current().isToken( TokIdentifier))
	{
		throw strus::runtime_error( _TXT("identifier expected in pattern matcher instead of %s inf feature definition after left arrow"), tokenName( lexer.current()));
	}
	std::string patternTypeName = lexer.current().value();
	lexer.next();

	// [2] Parse normalizer, if defined:
	if (!lexer.current().isToken( TokSemiColon) && !lexer.current().isToken( TokOpenCurlyBracket))
	{
		featuredef.parseNormalizer( lexer, textproc);
	}
	// [3] Parse feature options, if defined:
	analyzer::FeatureOptions featopt( parseFeatureOptions( lexer));

	// [4] Define the feature:
	switch (featureClass)
	{
		case FeatSearchIndexTerm:
			analyzer.addSearchIndexFeatureFromPatternMatch( 
				featureName, patternTypeName, featuredef.normalizer, priority, featopt);
			break;

		case FeatForwardIndexTerm:
			analyzer.addForwardIndexFeatureFromPatternMatch( 
				featureName, patternTypeName, featuredef.normalizer, priority, featopt);
			break;

		case FeatMetaData:
			if (priority)
			{
				throw strus::runtime_error( _TXT("no priority expected for meta data feature"));
			}
			if (featopt.opt())
			{
				throw strus::runtime_error( _TXT("no feature options expected for meta data feature"));
			}
			analyzer.defineMetaDataFromPatternMatch( 
				featureName, patternTypeName, featuredef.normalizer);
			break;

		case FeatAttribute:
			if (priority)
			{
				throw strus::runtime_error( _TXT("no priority expected for attribute feature"));
			}
			if (featopt.opt())
			{
				throw strus::runtime_error( _TXT("no feature options expected for attribute feature"));
			}
			analyzer.defineAttributeFromPatternMatch( 
				featureName, patternTypeName, featuredef.normalizer);
			break;

		case FeatPatternLexem:
			throw std::logic_error("cannot define pattern match lexem from pattern match result");
	}
	featuredef.release();
}

static void parseQueryPatternFeatureDef(
	ProgramLexer& lexer,
	QueryAnalyzerInstanceInterface& analyzer,
	const TextProcessorInterface* textproc,
	const std::string& featureName,
	int priority)
{
	FeatureDef featuredef;

	// [1] Parse pattern item name:
	if (!lexer.current().isToken( TokIdentifier))
	{
		throw strus::runtime_error( _TXT("identifier expected in pattern matcher instead of %s inf feature definition after left arrow"), tokenName( lexer.current()));
	}
	std::string patternTypeName = lexer.current().value();
	lexer.next();

	// [2] Parse normalizer, if defined:
	if (!lexer.current().isToken( TokSemiColon) && !lexer.current().isToken( TokOpenCurlyBracket))
	{
		featuredef.parseNormalizer( lexer, textproc);
	}
	// [3] Define the element:
	analyzer.addElementFromPatternMatch( featureName, patternTypeName, featuredef.normalizer, priority);
	featuredef.release();
}

static void parseDocumentFeatureDef(
	ProgramLexer& lexer,
	DocumentAnalyzerInstanceInterface& analyzer,
	const TextProcessorInterface* textproc,
	const std::string& featureName,
	int priority,
	DocumentFeatureClass featureClass)
{
	FeatureDef featuredef;
	// [1] Parse normalizer:
	featuredef.parseNormalizer( lexer, textproc);
	// [2] Parse tokenizer:
	featuredef.parseTokenizer( lexer, textproc);

	// [3] Parse feature options, if defined:
	analyzer::FeatureOptions featopt( parseFeatureOptions( lexer));

	// [4] Parse selection expression:
	std::string xpathexpr( parseSelectorExpression( lexer));

	switch (featureClass)
	{
		case FeatSearchIndexTerm:
			analyzer.addSearchIndexFeature(
				featureName, xpathexpr,
				featuredef.tokenizer.get(), featuredef.normalizer,
				priority, featopt);
			break;

		case FeatForwardIndexTerm:
			analyzer.addForwardIndexFeature(
				featureName, xpathexpr,
				featuredef.tokenizer.get(), featuredef.normalizer,
				priority, featopt);
			break;

		case FeatMetaData:
			if (priority)
			{
				throw strus::runtime_error( _TXT("no priority expected for meta data feature"));
			}
			if (featopt.opt())
			{
				throw strus::runtime_error( _TXT("no feature options expected for meta data feature"));
			}
			analyzer.defineMetaData(
				featureName, xpathexpr,
				featuredef.tokenizer.get(), featuredef.normalizer);
			break;

		case FeatAttribute:
			if (priority)
			{
				throw strus::runtime_error( _TXT("no priority expected for attribute feature"));
			}
			if (featopt.opt())
			{
				throw strus::runtime_error( _TXT("no feature options expected for attribute feature"));
			}
			analyzer.defineAttribute(
				featureName, xpathexpr,
				featuredef.tokenizer.get(), featuredef.normalizer);
			break;

		case FeatPatternLexem:
			if (featopt.opt())
			{
				throw strus::runtime_error( _TXT("no feature options expected for pattern lexem"));
			}
			analyzer.addPatternLexem(
				featureName, xpathexpr,
				featuredef.tokenizer.get(), featuredef.normalizer, priority);
			break;
	}
	featuredef.release();
}

static void parseQueryElementDef(
	ProgramLexer& lexer,
	QueryAnalyzerInstanceInterface& analyzer,
	const TextProcessorInterface* textproc,
	const std::string& featureName,
	int priority,
	QueryElementClass elementClass)
{
	FeatureDef featuredef;
	// [1] Parse normalizer:
	featuredef.parseNormalizer( lexer, textproc);
	// [2] Parse tokenizer:
	featuredef.parseTokenizer( lexer, textproc);

	// [4] Parse selection expression:
	if (!lexer.current().isString() && !lexer.current().isToken(TokIdentifier))
	{
		throw strus::runtime_error( _TXT("string or identifier expected as field name in query element definition"));
	}
	std::string fieldname( lexer.current().value());
	switch (elementClass)
	{
		case QueryElementTerm:
			analyzer.addElement(
				featureName, fieldname,
				featuredef.tokenizer.get(), featuredef.normalizer, priority);
			break;

		case QueryElementLexem:
			analyzer.addPatternLexem(
				featureName, fieldname,
				featuredef.tokenizer.get(), featuredef.normalizer, priority);
			break;
	}
	featuredef.release();
	lexer.next();
}

struct SectionHeader
{
	std::string name;
	std::vector<std::string> arg;

	SectionHeader()
		:name(),arg(){}
	SectionHeader( const std::string& name_, const std::vector<std::string>& arg_)
		:name(name_),arg(arg_){}
	SectionHeader( const SectionHeader& o)
		:name(o.name),arg(o.arg){}
};

static SectionHeader parseSectionHeader( ProgramLexer& lexer)
{
	SectionHeader rt;

	if (lexer.current().isToken(TokIdentifier))
	{
		rt.name = lexer.current().value();
		ProgramLexem cur = lexer.next();
		while (cur.isToken(TokIdentifier) || cur.isString())
		{
			rt.arg.push_back( cur.value());
			cur = lexer.next();
		}
		if (!cur.isToken(TokCloseSquareBracket))
		{
			throw strus::runtime_error( _TXT("close square bracket ']' expected instead of %s to close feature class section definition"), tokenName( lexer.current()));
		}
		lexer.next();
	}
	else
	{
		throw strus::runtime_error( _TXT("feature class identifier expected instead of %s after open square bracket '['"), tokenName( lexer.current()));
	}
	return rt;
}


static bool loadPatternMatcherProgramWithFeeder(
		PatternTermFeederInstanceInterface* feeder,
		PatternMatcherInstanceInterface* matcher,
		const std::string& source,
		ErrorBufferInterface* errorhnd)
{
	try
	{
		if (errorhnd->hasError()) throw std::runtime_error(_TXT("called load patter matcher program with error"));
		if (isPatternSerializerContent( source, errorhnd))
		{
			return loadPatternMatcherFromSerialization( source, feeder, matcher, errorhnd);
		}
		else
		{
			PatternMatcherProgramParser program( feeder, matcher, errorhnd);
			
			if (!program.load( source)) throw std::runtime_error( errorhnd->fetchError());
	
			if (!program.compile())
			{
				errorhnd->explain(_TXT("failed to compile pattern match program for analyzer output: %s"));
				return false;
			}
		}
		return true;
	}
	CATCH_ERROR_MAP_RETURN( _TXT("failed to load pattern match program with feeder: %s"), *errorhnd, false);
}

static bool loadPatternMatcherProgramWithLexer(
		PatternLexerInstanceInterface* lexer,
		PatternMatcherInstanceInterface* matcher,
		const std::string& source,
		ErrorBufferInterface* errorhnd)
{
	try
	{
		if (errorhnd->hasError()) throw std::runtime_error(_TXT("called load patter matcher program with error"));
		if (isPatternSerializerContent( source, errorhnd))
		{
			return loadPatternMatcherFromSerialization( source, lexer, matcher, errorhnd);
		}
		else
		{
			PatternMatcherProgramParser program( lexer, matcher, errorhnd);
			if (!program.load( source))
			{
				throw std::runtime_error( errorhnd->fetchError());
			}
			if (!program.compile())
			{
				errorhnd->explain(_TXT("failed to compile pattern match program: %s"));
				return false;
			}
		}
		return true;
	}
	CATCH_ERROR_MAP_RETURN( _TXT("failed to load pattern match program with lexer: %s"), *errorhnd, false);
}

template <class AnalyzerInstanceInterface>
static void parseAnalyzerPatternMatchProgramDef(
		ProgramLexer& lexer,
		AnalyzerInstanceInterface& analyzer,
		const TextProcessorInterface* textproc,
		const std::string& patternModuleName,
		const std::string& patternTypeName,
		ErrorBufferInterface* errorhnd)
{
	// Parse selection expressions if defined in case of pre-processing pattern matching
	std::vector<std::string> selectexprlist;
	if (lexer.current().isToken(TokOpenCurlyBracket))
	{
		do  {
			lexer.next();
			selectexprlist.push_back( parseSelectorExpression( lexer));
		} while (lexer.current().isToken(TokComma));
		if (!lexer.current().isToken(TokCloseCurlyBracket))
		{
			throw strus::runtime_error( _TXT("expected close curly bracket '}' instead of %s at end of pattern lexer selection expressions"), tokenName( lexer.current()));
		}
		lexer.next();
	}
	// Read source:
	std::vector<std::pair<std::string,std::string> > ptsources;
	std::string filename = parseSelectorExpression( lexer);
	std::string filepath = textproc->getResourceFilePath( filename);
	if (filepath.empty() && errorhnd->hasError())
	{
		throw strus::runtime_error(_TXT( "failed to evaluate pattern match file path '%s': %s"), filename.c_str(), errorhnd->fetchError());
	}
	std::string source;
	unsigned int ec = strus::readFile( filepath, source);
	if (ec)
	{
		throw strus::runtime_error(_TXT( "failed to read pattern match file '%s': %s"), filepath.c_str(), ::strerror(ec));
	}
	// Load the program:
	if (selectexprlist.empty())
	{
		const PatternMatcherInterface* matcher = textproc->getPatternMatcher( patternModuleName);
		const PatternTermFeederInterface* feeder = textproc->getPatternTermFeeder();
		if (!feeder || !matcher)
		{
			throw strus::runtime_error( _TXT("failed to create post proc pattern matching: %s"), errorhnd->fetchError());
		}
		strus::local_ptr<PatternTermFeederInstanceInterface> feederctx( feeder->createInstance());
		strus::local_ptr<PatternMatcherInstanceInterface> matcherctx( matcher->createInstance());
		if (!feederctx.get() || !matcherctx.get())
		{
			throw strus::runtime_error( _TXT("failed to create post proc pattern matching: %s"), errorhnd->fetchError());
		}
		if (!loadPatternMatcherProgramWithFeeder( feederctx.get(), matcherctx.get(), source, errorhnd))
		{
			throw strus::runtime_error( _TXT("failed to create post proc pattern matching: %s"), errorhnd->fetchError());
		}
		analyzer->definePatternMatcherPostProc( patternTypeName, matcherctx.get(), feederctx.get());
		matcherctx.release();
		feederctx.release();
		if (errorhnd->hasError())
		{
			throw strus::runtime_error( _TXT("failed to create post proc pattern matching: %s"), errorhnd->fetchError());
		}
	}
	else
	{
		const PatternLexerInterface* patternlexer = textproc->getPatternLexer( patternModuleName);
		const PatternMatcherInterface* patternmatcher = textproc->getPatternMatcher( patternModuleName);
		if (!patternlexer || !patternmatcher)
		{
			throw strus::runtime_error( _TXT("failed to create pre proc pattern matching: %s"), errorhnd->fetchError());
		}
		strus::local_ptr<PatternLexerInstanceInterface> lexerctx( patternlexer->createInstance());
		strus::local_ptr<PatternMatcherInstanceInterface> matcherctx( patternmatcher->createInstance());
		if (!lexerctx.get() || !matcherctx.get())
		{
			throw strus::runtime_error( _TXT("failed to create pre proc pattern matching: %s"), errorhnd->fetchError());
		}
		if (!loadPatternMatcherProgramWithLexer( lexerctx.get(), matcherctx.get(), source, errorhnd))
		{
			throw strus::runtime_error( _TXT("failed to create pre proc pattern matching: %s"), errorhnd->fetchError());
		}
		analyzer->definePatternMatcherPreProc( patternTypeName, matcherctx.get(), lexerctx.get(), selectexprlist);
		matcherctx.release();
		lexerctx.release();
		if (errorhnd->hasError())
		{
			throw strus::runtime_error( _TXT("failed to create pre proc pattern matching: %s"), errorhnd->fetchError());
		}
	}
}

static std::string parse_STRING( char const*& src)
{
	char eb = *src++;
	const char* start = src;
	for (; *src && *src != '\n' && *src != eb; ++src){}
	return std::string( start, src-start);
}

static void expandIncludes(
		const std::string& source,
		const TextProcessorInterface* textproc,
		std::set<std::string>& visited,
		std::vector<std::pair<std::string,std::string> >& contents,
		ErrorBufferInterface* errorhnd)
{
	char const* src = source.c_str();
	while ((unsigned char)*src <= 32) ++src;

	while (*src == '#' && std::memcmp( src, "#include", 8) == 0 && ((unsigned char)src[8] <= 32))
	{
		src+= 8;
		while ((unsigned char)*src <= 32) ++src;

		if (*src != '"') throw strus::runtime_error( _TXT("string expected as include file path"));
		std::string filename = parse_STRING( src);

		if (filename.empty()) throw strus::runtime_error( _TXT("include file name is empty"));
		std::string filepath = textproc->getResourceFilePath( filename);
		if (filepath.empty()) throw strus::runtime_error(_TXT("failed to find include file path '%s': %s"), filename.c_str(), errorhnd->fetchError());

		if (visited.find( filepath) == visited.end())
		{
			std::string include_source;
			unsigned int ec = strus::readFile( filepath, include_source);
			if (ec) throw strus::runtime_error(_TXT("failed to load include file '%s': %s"), filepath.c_str(), ::strerror( ec));

			visited.insert( filepath);
			expandIncludes( include_source, textproc, visited, contents, errorhnd);

			contents.push_back( std::pair<std::string,std::string>( filename, include_source));
		}
		while ((unsigned char)*src <= 32) ++src;
	}
}

static std::string getContentTypeElem( const char* key, const std::string& val)
{
	char const* ci;
	char const* ce;
	if (!key)
	{
		ci = val.c_str();
		ce = std::strchr( ci, ';');
		if (!ce) ce = std::strchr( ci, '\0');
		char const* eq = std::strchr( ci, '=');
		return (eq && eq < ce) ? std::string() : string_conv::trim( std::string( ci, ce-ci));
	}
	else
	{
		char keybuf[ 32];
		std::snprintf( keybuf, sizeof(keybuf), "%s=", key);

		ci = std::strstr( val.c_str(), keybuf);
		if (ci && (ci == val.c_str() || *(ci-1) == ',' || *(ci-1) == ';' || (unsigned char)*(ci-1) <= 32))
		{
			ci += std::strlen( keybuf);
			ce = std::strchr( ci, ',');
			if (!ce) ce = std::strchr( ci, ';');
			if (!ce) ce = std::strchr( ci, '\0');
			return string_conv::trim( std::string( ci, ce-ci));
		}
	}
	return std::string();
}

static void loadIncludes( DocumentAnalyzerInstanceInterface* analyzer, const TextProcessorInterface* textproc, const std::string& source, ErrorBufferInterface* errorhnd)
{
	std::set<std::string> visited;
	std::vector<std::pair<std::string,std::string> > include_contents;

	expandIncludes( source, textproc, visited, include_contents, errorhnd);
	std::vector<std::pair<std::string,std::string> >::const_iterator
		ci = include_contents.begin(), ce = include_contents.end();
	for (; ci != ce; ++ci)
	{
		if (!strus::loadDocumentAnalyzerProgramSource(
				analyzer, textproc, ci->second,
				false/*!allowIncludes*/, errorhnd))
		{
			throw strus::runtime_error(_TXT("failed to load include file '%s': %s"), ci->first.c_str(), errorhnd->fetchError());
		}
	}
}

static void loadIncludes( QueryAnalyzerInstanceInterface* analyzer, const TextProcessorInterface* textproc, const std::string& source, ErrorBufferInterface* errorhnd)
{
	std::set<std::string> visited;
	std::vector<std::pair<std::string,std::string> > include_contents;

	expandIncludes( source, textproc, visited, include_contents, errorhnd);
	std::vector<std::pair<std::string,std::string> >::const_iterator
		ci = include_contents.begin(), ce = include_contents.end();
	for (; ci != ce; ++ci)
	{
		if (!strus::loadQueryAnalyzerProgramSource(
				analyzer, textproc, ci->second,
				false/*!allowIncludes*/, errorhnd))
		{
			throw strus::runtime_error(_TXT("failed to load include file '%s': %s"), ci->first.c_str(), errorhnd->fetchError());
		}
	}
}

static analyzer::DocumentClass parseDocumentClass_( const std::string& value)
{
	std::string mimeType = getContentTypeElem( 0, value);
	if (mimeType.empty())
	{
		mimeType = getContentTypeElem( "content", value);
	}
	std::string encoding = getContentTypeElem( "charset", value);
	if (encoding.empty())
	{
		encoding = getContentTypeElem( "encoding", value);
	}
	std::string scheme = getContentTypeElem( "scheme", value);

	if (isEqual( mimeType,"xml") || isEqual( mimeType,"text/xml"))
	{
		mimeType = "application/xml";
	}
	else if (isEqual( mimeType,"json"))
	{
		mimeType = "application/json";
	}
	else if (isEqual( mimeType,"tsv"))
	{
		mimeType = "text/tab-separated-values";
	}
	return analyzer::DocumentClass( mimeType, encoding, scheme);
}

static analyzer::DocumentClass parseDocumentClass_( ProgramLexer& lexer)
{
	ProgramLexem cur = lexer.current();
	if (!cur.isString())
	{
		throw strus::runtime_error( _TXT("expected document class as string instead of %s at start of sub content definition"), tokenName( lexer.current()));
	}
	analyzer::DocumentClass rt = parseDocumentClass_( cur.value());
	lexer.next();
	return rt;
}

static void loadContentSection( ProgramLexer& lexer, const SectionHeader& header, DocumentAnalyzerInstanceInterface* analyzer, ErrorBufferInterface* errorhnd)
{
	ProgramLexem cur;
	if (!header.arg.empty())
	{
		throw strus::runtime_error( _TXT("no arguments expeted in section %s definition"), header.name.c_str());
	}
	do
	{
		// Define document content with different content-type:
		analyzer::DocumentClass documentClass = parseDocumentClass_( lexer);

		std::string xpathexpr( parseSelectorExpression( lexer));
		analyzer->defineSubContent( xpathexpr, documentClass);

		if (!lexer.current().isToken( TokSemiColon))
		{
			throw strus::runtime_error( _TXT("semicolon ';' instead of %s expected at end of %s declaration"), tokenName( lexer.current()), header.name.c_str());
		}
		lexer.next();
	}
	while (!lexer.current().end() && !lexer.current().isToken( TokOpenSquareBracket));
}

static void loadDocumentSection( ProgramLexer& lexer, const SectionHeader& header, DocumentAnalyzerInstanceInterface* analyzer, ErrorBufferInterface* errorhnd)
{
	if (!header.arg.empty())
	{
		throw strus::runtime_error( _TXT("no arguments expeted in section %s definition"), header.name.c_str());
	}
	do
	{
		if (!lexer.current().isToken(TokIdentifier))
		{
			throw strus::runtime_error( _TXT("feature type name (identifier) instead of %s expected at start of a %s declaration"), tokenName( lexer.current()), header.name.c_str());
		}
		std::string identifier = lexer.current().value();
		if (!lexer.next().isToken(TokAssign)) throw strus::runtime_error( _TXT("expected assignment operator '=' instead of %s after identifier of sub document definition"), tokenName( lexer.current()));
		lexer.next();

		std::string xpathexpr( parseSelectorExpression( lexer));
		analyzer->defineSubDocument( identifier, xpathexpr);

		if (!lexer.current().isToken( TokSemiColon))
		{
			throw strus::runtime_error( _TXT("semicolon ';' expected instead of %s at end of %s declaration"), tokenName( lexer.current()), header.name.c_str());
		}
		lexer.next();
	}
	while (!lexer.current().end() && !lexer.current().isToken( TokOpenSquareBracket));
}

static void loadAggregatorSection( ProgramLexer& lexer, const SectionHeader& header, DocumentAnalyzerInstanceInterface* analyzer, const TextProcessorInterface* textproc, ErrorBufferInterface* errorhnd)
{
	if (!header.arg.empty())
	{
		throw strus::runtime_error( _TXT("no arguments expeted in section %s definition"), header.name.c_str());
	}
	do
	{
		if (!lexer.current().isToken(TokIdentifier))
		{
			throw strus::runtime_error( _TXT("feature type name (identifier) expected instead of %s at start of a %s declaration"), tokenName( lexer.current()), header.name.c_str());
		}
		std::string identifier = lexer.current().value();
		if (!lexer.next().isToken(TokAssign)) throw strus::runtime_error( _TXT("expected assignment operator '=' instead of %s after identifier of sub document definition"), tokenName( lexer.current()));
		lexer.next();

		FunctionConfig cfg = parseAggregatorFunctionConfig( lexer);

		const AggregatorFunctionInterface* sf = textproc->getAggregator( cfg.name());
		if (!sf) throw strus::runtime_error(_TXT( "unknown aggregator function '%s'"), cfg.name().c_str());
		
		strus::local_ptr<AggregatorFunctionInstanceInterface> statfunc( sf->createInstance( cfg.args()));
		if (!statfunc.get()) throw strus::runtime_error(_TXT( "failed to create instance of aggregator function '%s'"), cfg.name().c_str());

		analyzer->defineAggregatedMetaData( identifier, statfunc.get());
		statfunc.release();

		if (!lexer.current().isToken( TokSemiColon))
		{
			throw strus::runtime_error( _TXT("semicolon ';' instead of %s expected at end of %s declaration"), tokenName( lexer.current()), header.name.c_str());
		}
		lexer.next();
	}
	while (!lexer.current().end() && !lexer.current().isToken( TokOpenSquareBracket));
}

template <class AnalyzerInstanceInterface>
static void loadPatternMatchSection( ProgramLexer& lexer, const SectionHeader& header, AnalyzerInstanceInterface* analyzer, const TextProcessorInterface* textproc, ErrorBufferInterface* errorhnd)
{
	std::string patternModuleName = header.arg.empty() ? std::string() : header.arg[0];
	if (header.arg.size() > 2)
	{
		throw strus::runtime_error( _TXT("too many arguments in section %s definition"), header.name.c_str());
	}
	do
	{
		if (!lexer.current().isToken(TokIdentifier))
		{
			throw strus::runtime_error( _TXT("feature type name (identifier) expected at start of a %s declaration"), header.name.c_str());
		}
		std::string identifier = lexer.current().value();
		if (!lexer.next().isToken(TokAssign)) throw strus::runtime_error( _TXT("expected assignment operator '=' instead of %s after identifier of sub document definition"), tokenName( lexer.current()));
		lexer.next();
		parseAnalyzerPatternMatchProgramDef( lexer, analyzer, textproc, patternModuleName, identifier, errorhnd);
		if (!lexer.current().isToken( TokSemiColon))
		{
			throw strus::runtime_error( _TXT("semicolon ';' instead of %s expected at end of %s declaration"), tokenName( lexer.current()), header.name.c_str());
		}
		lexer.next();
	}
	while (!lexer.current().end() && !lexer.current().isToken( TokOpenSquareBracket));
}

static void loadFeatureSection( ProgramLexer& lexer, const DocumentFeatureClass& featclass, DocumentAnalyzerInstanceInterface* analyzer, const TextProcessorInterface* textproc, ErrorBufferInterface* errorhnd)
{
	do
	{
		if (!lexer.current().isToken(TokIdentifier))
		{
			throw strus::runtime_error( _TXT("feature type name (identifier) expected instead of %s at start of a %s declaration"), tokenName( lexer.current()), "feature");
		}
		std::string identifier = lexer.current().value();
		lexer.next();
		int priority = 0;
		if (lexer.current().isToken(TokExp))
		{
			if (!lexer.next().isToken(TokInteger))
			{
				throw strus::runtime_error( _TXT("priority value (integer) expected intead of %s after exp '^'"), tokenName( lexer.current()));
			}
			priority = numstring_conv::touint( lexer.current().value(), std::numeric_limits<int>::max());
			lexer.next();
		}
		if (lexer.current().isToken(TokAssign))
		{
			lexer.next();
			parseDocumentFeatureDef( lexer, *analyzer, textproc, identifier, priority, featclass);
		}
		else if (lexer.current().isToken( TokLeftArrow))
		{
			lexer.next();
			parseDocumentPatternFeatureDef( lexer, *analyzer, textproc, identifier, priority, featclass);
		}
		else
		{
			throw strus::runtime_error( _TXT("assignment operator '=' or '<-' instead of %s expected after set identifier in a %s declaration"), tokenName( lexer.current()), "feature");
		}
		if (!lexer.current().isToken( TokSemiColon))
		{
			throw strus::runtime_error( _TXT("semicolon ';' instead of %s expected at end of %s declaration"), tokenName( lexer.current()), "feature");
		}
		lexer.next();
	}
	while (!lexer.current().end() && !lexer.current().isToken( TokOpenSquareBracket));
}

static void loadQueryElementSection( ProgramLexer& lexer, const QueryElementClass& featclass, QueryAnalyzerInstanceInterface* analyzer, const TextProcessorInterface* textproc, ErrorBufferInterface* errorhnd)
{
	do
	{
		if (!lexer.current().isToken(TokIdentifier))
		{
			throw strus::runtime_error( _TXT("feature type name (identifier) expected instead of %s at start of a %s declaration"), tokenName( lexer.current()), "feature");
		}
		std::string identifier = lexer.current().value();
		lexer.next();
		int priority = 0;
		if (lexer.current().isToken(TokExp))
		{
			if (!lexer.next().isToken(TokInteger))
			{
				throw strus::runtime_error( _TXT("priority value (integer) expected intead of %s after exp '^'"), tokenName( lexer.current()));
			}
			priority = numstring_conv::touint( lexer.current().value(), std::numeric_limits<int>::max());
			lexer.next();
		}
		if (lexer.current().isToken(TokAssign))
		{
			lexer.next();
			parseQueryElementDef( lexer, *analyzer, textproc, identifier, priority, featclass);
		}
		else if (lexer.current().isToken( TokLeftArrow))
		{
			if (featclass != QueryElementTerm) throw strus::runtime_error(_TXT("pattern feature definitions ('<-') only allowed in \"SearchTerm\" section"));
			lexer.next();
			parseQueryPatternFeatureDef( lexer, *analyzer, textproc, identifier, priority);
		}
		else
		{
			throw strus::runtime_error( _TXT("assignment operator '=' or '<-' instead of %s expected after set identifier in a %s declaration"), tokenName( lexer.current()), "feature");
		}
		if (!lexer.current().isToken( TokSemiColon))
		{
			throw strus::runtime_error( _TXT("semicolon ';' instead of %s expected at end of %s declaration"), tokenName( lexer.current()), "feature");
		}
		lexer.next();
	}
	while (!lexer.current().end() && !lexer.current().isToken( TokOpenSquareBracket));
}

bool strus::isDocumentAnalyzerProgramSource(
		const std::string& source,
		ErrorBufferInterface* errorhnd)
{
	try
	{
		// Skip includes:
		char const* src = source.c_str();
		while ((unsigned char)*src < 32) ++src;
		while (*src == '#' && std::memcmp( src, "#include", 8) == 0 && ((unsigned char)src[8] <= 32))
		{
			while (*src != '\n') ++src;
			while ((unsigned char)*src < 32) ++src;
		}
		// Check first tokens:
		ProgramLexer lexer( src, g_eolncomment, g_tokens, g_errtokens, errorhnd);
		if (lexer.next().isToken( TokOpenSquareBracket))
		{
			lexer.next();
			SectionHeader header = parseSectionHeader( lexer);
			return true;
		}
		else if (lexer.current().isToken(TokIdentifier) && lexer.next().isToken(TokAssign))
		{
			return true;
		}
		else if (!*src)
		{
			 return true;
		}
		else
		{
			return false;
		}
	}
	CATCH_ERROR_MAP_RETURN( _TXT("error checking if program is a document analyzer source: %s"), *errorhnd, false);
}

bool strus::isDocumentAnalyzerProgramFile(
		const TextProcessorInterface* textproc,
		const std::string& filename,
		ErrorBufferInterface* errorhnd)
{
	try
	{
		std::string filepath = textproc->getResourceFilePath( filename);
		if (filepath.empty() && errorhnd->hasError())
		{
			throw strus::runtime_error(_TXT( "failed to evaluate file path '%s': %s"), filename.c_str(), errorhnd->fetchError());
		}
		std::string source;
		unsigned int ec = strus::readFile( filepath, source);
		if (ec)
		{
			throw strus::runtime_error(_TXT( "failed to read file '%s': %s"), filepath.c_str(), ::strerror(ec));
		}
		return isDocumentAnalyzerProgramSource( source, errorhnd);
	}
	CATCH_ERROR_MAP_RETURN( _TXT("error determining the type of an analyzer program file: %s"), *errorhnd, false);
}

static void reportErrorWithLocation( ErrorBufferInterface* errorhnd, ProgramLexer& lexer, const char* msg)
{
	try
	{
		std::string errorlocation = lexer.currentLocationString( -30, 80, "<!>");
		std::string errormsg = strus::string_format(
			_TXT("error in source on line %d (at %s): %s"),
			(int)lexer.lineno(), errorlocation.c_str(), msg);
		errorhnd->report( ErrorCodeSyntax, "%s", errormsg.c_str());
	}
	catch (const std::bad_alloc&)
	{
		errorhnd->report( ErrorCodeOutOfMem, _TXT("out of memory handling error"));
	}
}

bool strus::loadDocumentAnalyzerProgramSource( DocumentAnalyzerInstanceInterface* analyzer, const TextProcessorInterface* textproc, const std::string& source, bool allowIncludes, ErrorBufferInterface* errorhnd)
{
	ProgramLexer lexer( source.c_str(), g_eolncomment, g_tokens, g_errtokens, errorhnd);
	try
	{
		if (errorhnd->hasError()) return false;
		if (allowIncludes)
		{
			loadIncludes( analyzer, textproc, source, errorhnd);
		}
		if (!lexer.next().isToken( TokOpenSquareBracket))
		{
			loadFeatureSection( lexer, FeatSearchIndexTerm, analyzer, textproc, errorhnd);
		}
		while (lexer.current().isToken( TokOpenSquareBracket))
		{
			lexer.next();
			SectionHeader header = parseSectionHeader( lexer);
			if (lexer.current().isToken( TokOpenSquareBracket) || lexer.current().isEof()) continue;

			if (isEqual( header.name, "Content"))
			{
				loadContentSection( lexer, header, analyzer, errorhnd);
			}
			else if (isEqual( header.name, "Document"))
			{
				loadDocumentSection( lexer, header, analyzer, errorhnd);
			}
			else if (isEqual( header.name, "Aggregator"))
			{
				loadAggregatorSection( lexer, header, analyzer, textproc, errorhnd);
			}
			else if (isEqual( header.name, "PatternMatch"))
			{
				loadPatternMatchSection( lexer, header, analyzer, textproc, errorhnd);
			}
			else
			{
				if (!header.arg.empty())
				{
					throw strus::runtime_error( _TXT("no arguments expected in section %s definition"), header.name.c_str());
				}
				DocumentFeatureClass featclass;
				if (getDocumentFeatureClassFromName( featclass, header.name))
				{
					loadFeatureSection( lexer, featclass, analyzer, textproc, errorhnd);
				}
				else
				{
					throw strus::runtime_error( _TXT("unknown document analyzer program section name '%s'"), header.name.c_str());
					continue;
				}
			}
		}
		if (!lexer.current().end())
		{
			throw strus::runtime_error( _TXT("unexpected token (%s) in document analyzer program"), tokenName( lexer.current()));
		}
		return !errorhnd->hasError();
	}
	catch (const std::bad_alloc& err)
	{
		errorhnd->report( ErrorCodeOutOfMem, _TXT("out of memory loading document analyzer program source"));
		return false;
	}
	catch (const std::runtime_error& err)
	{
		reportErrorWithLocation( errorhnd, lexer, err.what());
		return false;
	}
}

bool strus::loadDocumentAnalyzerProgramFile( DocumentAnalyzerInstanceInterface* analyzer, const TextProcessorInterface* textproc, const std::string& filename, ErrorBufferInterface* errorhnd)
{
	try
	{
		if (filename.empty()) throw strus::runtime_error( _TXT("program file name is empty"));
		std::string filepath = textproc->getResourceFilePath( filename);
		if (filepath.empty()) throw std::runtime_error(_TXT("failed to find program path"));
		std::string content;
		int ec = strus::readFile( filepath, content);
		if (ec) throw strus::runtime_error(_TXT("failed to read program file: %s"), ::strerror(ec));
		return strus::loadDocumentAnalyzerProgramSource( analyzer, textproc, content, true/*allowIncludes*/, errorhnd);
	}
	CATCH_ERROR_MAP_RETURN( _TXT("error loading document analyzer program file: %s"), *errorhnd, false);
}

bool strus::loadDocumentAnalyzerMapSource(
		DocumentAnalyzerMapInterface* analyzermap,
		const TextProcessorInterface* textproc,
		const std::string& source,
		ErrorBufferInterface* errorhnd)
{
	ProgramLexer lexer( source.c_str(), g_eolncomment, g_tokens, g_errtokens, errorhnd);
	try
	{
		if (errorhnd->hasError()) return false;
		while (!lexer.next().isEof())
		{
			analyzer::DocumentClass doctype;
			std::string programName;

			while (lexer.current().isToken( TokIdentifier))
			{
				std::string cmd = lexer.current().value();
				lexer.next();
				if (strus::caseInsensitiveEquals( cmd, "analyze"))
				{
					if (doctype.defined()) throw std::runtime_error(_TXT("analyze <document class> defined twice (missing semicolon ';'?)"));
					doctype = parseDocumentClass_( lexer);
				}
				else if (strus::caseInsensitiveEquals( cmd, "program"))
				{
					if (!programName.empty()) throw std::runtime_error(_TXT("analyze <document class> defined twice (missing semicolon ';'?)"));
					programName = parseFileName( lexer);
				}
				else
				{
					throw std::runtime_error( _TXT("unexpected token, 'analyze' or 'program' expected"));
				}
			}
			if (!lexer.current().isToken( TokSemiColon))
			{
				throw std::runtime_error( _TXT("unexpected token, semicolon expected after analyzer map declaration"));
			}
			strus::Reference<DocumentAnalyzerInstanceInterface> analyzer( analyzermap->createAnalyzer( doctype.mimeType(), doctype.scheme()));
			if (!analyzer.get()) throw std::runtime_error( errorhnd->fetchError());
			if (!loadDocumentAnalyzerProgramFile( analyzer.get(), textproc, programName, errorhnd))
			{
				throw std::runtime_error( errorhnd->fetchError());
			}
			analyzermap->addAnalyzer( doctype.mimeType(), doctype.scheme(), analyzer.get());
			if (errorhnd->hasError())
			{
				throw std::runtime_error( errorhnd->fetchError());
			}
			analyzer.release();
		}
		return true;
	}
	catch (const std::bad_alloc& err)
	{
		errorhnd->report( ErrorCodeOutOfMem, _TXT("out of memory loading analyzer map source"));
		return false;
	}
	catch (const std::runtime_error& err)
	{
		reportErrorWithLocation( errorhnd, lexer, err.what());
		return false;
	}
}

bool strus::loadDocumentAnalyzerMapFile( DocumentAnalyzerMapInterface* analyzermap, const TextProcessorInterface* textproc, const std::string& filename, ErrorBufferInterface* errorhnd)
{
	try
	{
		if (filename.empty()) throw strus::runtime_error( _TXT("program file name is empty"));
		std::string filepath = textproc->getResourceFilePath( filename);
		if (filepath.empty()) throw std::runtime_error(_TXT("failed to find program path"));
		std::string content;
		int ec = strus::readFile( filepath, content);
		if (ec) throw strus::runtime_error(_TXT("failed to read program file: %s"), ::strerror(ec));
		return strus::loadDocumentAnalyzerMapSource( analyzermap, textproc, content, errorhnd);
	}
	CATCH_ERROR_ARG1_MAP_RETURN( _TXT("error loading document analyzer map file '%s': %s"), filename.c_str(), *errorhnd, false);
}


bool strus::loadQueryAnalyzerProgramSource( QueryAnalyzerInstanceInterface* analyzer, const TextProcessorInterface* textproc, const std::string& source, bool allowIncludes, ErrorBufferInterface* errorhnd)
{
	ProgramLexer lexer( source.c_str(), g_eolncomment, g_tokens, g_errtokens, errorhnd);
	try
	{
		if (errorhnd->hasError()) return false;
		if (allowIncludes)
		{
			loadIncludes( analyzer, textproc, source, errorhnd);
		}
		if (!lexer.next().isToken( TokOpenSquareBracket))
		{
			loadQueryElementSection( lexer, QueryElementTerm, analyzer, textproc, errorhnd);
		}
		while (lexer.current().isToken( TokOpenSquareBracket))
		{
			lexer.next();
			SectionHeader header = parseSectionHeader( lexer);
			if (lexer.current().isToken( TokOpenSquareBracket) || lexer.current().isEof()) continue;

			if (isEqual( header.name, "PatternMatch"))
			{
				loadPatternMatchSection( lexer, header, analyzer, textproc, errorhnd);
			}
			else
			{
				if (!header.arg.empty())
				{
					throw strus::runtime_error( _TXT("no arguments expected in section %s definition"), header.name.c_str());
				}
				QueryElementClass elemclass;
				if (getQueryElementClassFromName( elemclass, header.name))
				{
					loadQueryElementSection( lexer, elemclass, analyzer, textproc, errorhnd);
				}
				else
				{
					throw strus::runtime_error( _TXT("unknown query analyzer program section name '%s'"), header.name.c_str());
					continue;
				}
			}
		}
		if (!lexer.current().end())
		{
			throw strus::runtime_error( _TXT("unexpected token (%s) in query analyzer program"), tokenName( lexer.current()));
		}
		return !errorhnd->hasError();
	}
	catch (const std::bad_alloc& err)
	{
		errorhnd->report( ErrorCodeOutOfMem, _TXT("out of memory loading query analyzer program source"));
		return false;
	}
	catch (const std::runtime_error& err)
	{
		reportErrorWithLocation( errorhnd, lexer, err.what());
		return false;
	}
}

bool strus::loadQueryAnalyzerProgramFile(
		QueryAnalyzerInstanceInterface* analyzer,
		const TextProcessorInterface* textproc,
		const std::string& filename,
		ErrorBufferInterface* errorhnd)
{
	try
	{
		if (filename.empty()) throw strus::runtime_error( _TXT("program file name is empty"));
		std::string filepath = textproc->getResourceFilePath( filename);
		if (filepath.empty()) throw std::runtime_error(_TXT("failed to find program path"));
		std::string content;
		int ec = strus::readFile( filepath, content);
		if (ec) throw strus::runtime_error(_TXT("failed to read program file: %s"), ::strerror(ec));
		return strus::loadQueryAnalyzerProgramSource( analyzer, textproc, content, true/*allowIncludes*/, errorhnd);
	}
	CATCH_ERROR_MAP_RETURN( _TXT("error loading query analyzer program file: %s"), *errorhnd, false);
}

analyzer::DocumentClass strus::parseDocumentClass(
		const std::string& src,
		ErrorBufferInterface* errorhnd)
{
	try
	{
		if (errorhnd->hasError()) return analyzer::DocumentClass();
		return parseDocumentClass_( src);
	}
	CATCH_ERROR_ARG1_MAP_RETURN( _TXT("error parsing document class '%s': %s"), src.c_str(), *errorhnd, analyzer::DocumentClass());
}




