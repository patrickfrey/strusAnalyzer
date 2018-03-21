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
#include "strus/documentAnalyzerInterface.hpp"
#include "strus/textProcessorInterface.hpp"
#include "strus/base/programLexer.hpp"
#include "strus/base/fileio.hpp"
#include "strus/base/string_conv.hpp"
#include "strus/base/local_ptr.hpp"
#include "strus/reference.hpp"
#include "strus/summarizerFunctionInstanceInterface.hpp"
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
	",",
	":",
	";",
	"<-",
	"[/][^\\;,\\{]*",
	NULL
};
static const char* g_errtokens[] = {
	"[0-9][0-9]*[a-zA-Z_]",
	NULL
};
static const char* g_eolncomment = "#";


enum FeatureClass
{
	FeatSearchIndexTerm,
	FeatForwardIndexTerm,
	FeatMetaData,
	FeatAttribute,
	FeatPatternLexem,
	FeatPatternMatch,
	FeatSubDocument,
	FeatSubContent,
	FeatAggregator
};

static bool isEqual( const std::string& id, const char* idstr)
{
	char const* si = id.c_str();
	char const* di = idstr;
	for (; *si && *di && ((*si|32) == (*di|32)); ++si,++di){}
	return !*si && !*di;
}

static FeatureClass featureClassFromName( const std::string& name)
{
	if (isEqual( name, "SearchIndex"))
	{
		return FeatSearchIndexTerm;
	}
	if (isEqual( name, "ForwardIndex"))
	{
		return FeatForwardIndexTerm;
	}
	if (isEqual( name, "MetaData"))
	{
		return FeatMetaData;
	}
	if (isEqual( name, "Attribute"))
	{
		return FeatAttribute;
	}
	if (isEqual( name, "PatternLexem"))
	{
		return FeatPatternLexem;
	}
	if (isEqual( name, "PatternMatch"))
	{
		return FeatPatternMatch;
	}
	if (isEqual( name, "Document"))
	{
		return FeatSubDocument;
	}
	if (isEqual( name, "Content"))
	{
		return FeatSubContent;
	}
	if (isEqual( name, "Aggregator"))
	{
		return FeatAggregator;
	}
	throw strus::runtime_error( _TXT( "illegal feature class name '%s' (expected one of {SearchIndex, ForwardIndex, MetaData, Attribute, Document, Aggregator})"), name.c_str());
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
			throw strus::runtime_error( _TXT("unexpected token in argument list: %s"), cur.value().c_str());
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
				throw strus::runtime_error( _TXT("comma ',' as argument separator or close oval bracket ')' expected at end of %s argument list"), functype);
			}
			lexer.next();
		}
	}
	else
	{
		throw strus::runtime_error( _TXT("%s definition (identifier) expected"), functype);
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
					throw strus::runtime_error( _TXT("assign '=' expected after open curly brackets '{' and option identifier"));
				}
				cur = lexer.next();
				if (cur.isString() || cur.isToken( TokIdentifier))
				{
					optval = cur.value();
				}
				else
				{
					throw strus::runtime_error( _TXT("identifier or string expected as option value"));
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
						throw strus::runtime_error( _TXT("'pred' or 'succ' expected as 'position' option value instead of '%s'"), optval.c_str());
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
				throw strus::runtime_error( _TXT("bad option name token '%s'"), cur.value().c_str());
			}
		}
		while (cur.isToken( TokComma));

		if (!cur.isToken( TokCloseCurlyBracket))
		{
			throw strus::runtime_error( _TXT("close curly bracket '}' expected at end of option list"));
		}
		lexer.next();
	}
	return rt;
}

static std::string parseSelectorExpression( ProgramLexer& lexer)
{
	ProgramLexem cur = lexer.current();
	if (cur.isString() || cur.isToken( TokPath))
	{
		std::string rt = cur.value();
		lexer.next();
		return rt;
	}
	else
	{
		throw strus::runtime_error( _TXT("bad selector expression token '%s'"), cur.value().c_str());
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
	DocumentAnalyzerInterface& analyzer,
	const TextProcessorInterface* textproc,
	const std::string& featureName,
	FeatureClass featureClass)
{
	FeatureDef featuredef;

	// [1] Parse pattern item name:
	ProgramLexem cur = lexer.current();
	if (!cur.isToken( TokIdentifier))
	{
		throw strus::runtime_error( "%s", _TXT("identifier expected in pattern matcher feature definition after left arrow"));
	}
	std::string patternTypeName = cur.value();

	// [2] Parse normalizer, if defined:
	if (!cur.isToken( TokSemiColon) && !cur.isToken( TokOpenCurlyBracket))
	{
		featuredef.parseNormalizer( lexer, textproc);
	}
	// [3] Parse feature options, if defined:
	analyzer::FeatureOptions featopt( parseFeatureOptions( lexer));

	switch (featureClass)
	{
		case FeatSearchIndexTerm:
			analyzer.addSearchIndexFeatureFromPatternMatch( 
				featureName, patternTypeName, featuredef.normalizer, featopt);
			break;

		case FeatForwardIndexTerm:
			analyzer.addForwardIndexFeatureFromPatternMatch( 
				featureName, patternTypeName, featuredef.normalizer, featopt);
			break;

		case FeatMetaData:
			if (featopt.opt())
			{
				throw strus::runtime_error( _TXT("no feature options expected for meta data feature"));
			}
			analyzer.defineMetaDataFromPatternMatch( 
				featureName, patternTypeName, featuredef.normalizer);
			break;

		case FeatAttribute:
			if (featopt.opt())
			{
				throw strus::runtime_error( _TXT("no feature options expected for attribute feature"));
			}
			analyzer.defineAttributeFromPatternMatch( 
				featureName, patternTypeName, featuredef.normalizer);
			break;

		case FeatPatternLexem:
			throw std::logic_error("cannot define pattern match lexem from pattern match result");

		case FeatPatternMatch:
			throw std::logic_error("illegal call of parse feature definition for pattern match program definition");

		case FeatSubDocument:
			throw std::logic_error("illegal call of parse feature definition for sub document");

		case FeatSubContent:
			throw std::logic_error("illegal call of parse feature definition for sub content");

		case FeatAggregator:
			throw std::logic_error("illegal call of parse feature definition for aggregator");
	}
	featuredef.release();
}

static void parseDocumentFeatureDef(
	ProgramLexer& lexer,
	DocumentAnalyzerInterface& analyzer,
	const TextProcessorInterface* textproc,
	const std::string& featureName,
	FeatureClass featureClass)
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
				featopt);
			break;

		case FeatForwardIndexTerm:
			analyzer.addForwardIndexFeature(
				featureName, xpathexpr,
				featuredef.tokenizer.get(), featuredef.normalizer,
				featopt);
			break;

		case FeatMetaData:
			if (featopt.opt())
			{
				throw strus::runtime_error( _TXT("no feature options expected for meta data feature"));
			}
			analyzer.defineMetaData(
				featureName, xpathexpr,
				featuredef.tokenizer.get(), featuredef.normalizer);
			break;

		case FeatAttribute:
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
				featuredef.tokenizer.get(), featuredef.normalizer);
			break;

		case FeatPatternMatch:
			throw std::logic_error("illegal call of parse feature definition for pattern match program definition");
		case FeatSubDocument:
			throw std::logic_error("illegal call of parse feature definition for sub document");
		case FeatSubContent:
			throw std::logic_error("illegal call of parse feature definition for sub content");
		case FeatAggregator:
			throw std::logic_error("illegal call of parse feature definition for aggregator");
	}
	featuredef.release();
}

static FeatureClass parseFeatureClassDef( ProgramLexer& lexer, std::string& domainid)
{
	FeatureClass rt = FeatSearchIndexTerm;
	ProgramLexem cur = lexer.current();

	if (cur.isToken(TokOpenSquareBracket))
	{
		cur = lexer.next();
		if (!cur.isToken(TokIdentifier))
		{
			throw strus::runtime_error( _TXT("feature class identifier expected after open square bracket '['"));
		}
		rt = featureClassFromName( cur.value());
		cur = lexer.next();
		if (rt == FeatPatternMatch && cur.isToken(TokIdentifier))
		{
			domainid = cur.value();
			cur = lexer.next();
		}
		if (!cur.isToken(TokCloseSquareBracket))
		{
			throw strus::runtime_error( _TXT("close square bracket ']' expected to close feature class section definition"));
		}
		lexer.next();
	}
	return rt;
}

enum StatementType
{
	AssignNormalizedTerm,
	AssignPatternResult
};

static bool loadPatternMatcherProgramWithFeeder(
		PatternTermFeederInstanceInterface* feeder,
		PatternMatcherInstanceInterface* matcher,
		const std::string& source,
		ErrorBufferInterface* errorhnd,
		std::vector<std::string>& warnings)
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
			warnings = program.warnings();
		}
		return true;
	}
	catch (const std::runtime_error& e)
	{
		errorhnd->report( ErrorCodeOutOfMem, _TXT("failed to load pattern match program (for analyzer output): %s"), e.what());
		return false;
	}
	catch (const std::bad_alloc&)
	{
		errorhnd->report( ErrorCodeRuntimeError, _TXT("out of memory loading pattern match program (for analyzer output)"));
		return false;
	}
}

static bool loadPatternMatcherProgramWithLexer(
		PatternLexerInstanceInterface* lexer,
		PatternMatcherInstanceInterface* matcher,
		const std::string& source,
		ErrorBufferInterface* errorhnd,
		std::vector<std::string>& warnings)
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
			warnings = program.warnings();
		}
		return true;
	}
	catch (const std::runtime_error& e)
	{
		errorhnd->report( ErrorCodeOutOfMem, _TXT("failed to load pattern match program: %s"), e.what());
		return false;
	}
	catch (const std::bad_alloc&)
	{
		errorhnd->report( ErrorCodeRuntimeError, _TXT("out of memory loading pattern match program"));
		return false;
	}
}

template <class AnalyzerInterface>
static void parseAnalyzerPatternMatchProgramDef(
		ProgramLexer& lexer,
		AnalyzerInterface& analyzer,
		const TextProcessorInterface* textproc,
		const std::string& patternModuleName,
		const std::string& patternTypeName,
		std::vector<std::string>& warnings,
		ErrorBufferInterface* errorhnd)
{
	std::vector<std::string> selectexprlist;
	ProgramLexem cur = lexer.current();

	if (cur.isToken(TokOpenCurlyBracket))
	{
		do
		{
			cur = lexer.next();
			selectexprlist.push_back( parseSelectorExpression( lexer));
		} while (cur.isToken(TokComma));
		if (!cur.isToken(TokCloseCurlyBracket))
		{
			throw strus::runtime_error( "%s", _TXT("expected close curly bracket '}' at end of pattern lexer selection expressions"));
		}
		cur = lexer.next();
	}
	std::vector<std::pair<std::string,std::string> > ptsources;
	std::string filename = parseSelectorExpression( lexer);
	std::string filepath = textproc->getResourcePath( filename);
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
		if (!loadPatternMatcherProgramWithFeeder( feederctx.get(), matcherctx.get(), source, errorhnd, warnings))
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
		if (!loadPatternMatcherProgramWithLexer( lexerctx.get(), matcherctx.get(), source, errorhnd, warnings))
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

		if (*src != '"') throw strus::runtime_error( "%s", _TXT("string expected as include file path"));
		std::string filename = parse_STRING( src);

		if (filename.empty()) throw strus::runtime_error( "%s", _TXT("include file name is empty"));
		std::string filepath = textproc->getResourcePath( filename);
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

static analyzer::DocumentClass parseDocumentClass( ProgramLexer& lexer)
{
	ProgramLexem cur = lexer.current();
	if (!cur.isString())
	{
		throw strus::runtime_error( _TXT("expected document class as string at start of sub content definition"));
	}
	std::string mimeType = getContentTypeElem( 0, cur.value());
	if (mimeType.empty())
	{
		mimeType = getContentTypeElem( "content", cur.value());
	}
	std::string encoding = getContentTypeElem( "charset", cur.value());
	if (encoding.empty())
	{
		encoding = getContentTypeElem( "encoding", cur.value());
	}
	std::string scheme = getContentTypeElem( "scheme", cur.value());

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
	return analyzer::DocumentClass( mimeType, encoding);
}

bool strus::loadDocumentAnalyzerProgram( DocumentAnalyzerInterface* analyzer, const TextProcessorInterface* textproc, const std::string& source, bool allowIncludes, std::vector<std::string>& warnings, ErrorBufferInterface* errorhnd)
{
	ProgramLexer lexer( source.c_str(), g_eolncomment, g_tokens, g_errtokens, errorhnd);
	try
	{
		if (errorhnd->hasError()) return false;
		if (allowIncludes)
		{
			std::set<std::string> visited;
			std::vector<std::pair<std::string,std::string> > include_contents;

			expandIncludes( source, textproc, visited, include_contents, errorhnd);
			std::vector<std::pair<std::string,std::string> >::const_iterator
				ci = include_contents.begin(), ce = include_contents.end();
			for (; ci != ce; ++ci)
			{
				if (!strus::loadDocumentAnalyzerProgram(
						analyzer, textproc, ci->second,
						false/*!allowIncludes*/, warnings, errorhnd))
				{
					throw strus::runtime_error(_TXT("failed to load include file '%s': %s"), ci->first.c_str(), errorhnd->fetchError());
				}
			}
		}
		FeatureClass featclass = FeatSearchIndexTerm;
		std::string featclassid;

		ProgramLexem cur = lexer.next();
		while (!cur.end())
		{
			if (lexer.current().isToken( TokOpenSquareBracket))
			{
				featclass = parseFeatureClassDef( lexer, featclassid);
				cur = lexer.current();
				continue;
			}
			if (featclass == FeatSubContent)
			{
				// Define document content with different content-type:
				analyzer::DocumentClass documentClass = parseDocumentClass( lexer);
				cur = lexer.next();

				std::string xpathexpr( parseSelectorExpression( lexer));
				analyzer->defineSubContent( xpathexpr, documentClass);

				if (!lexer.current().isToken(TokSemiColon))
				{
					throw strus::runtime_error( _TXT("semicolon ';' expected at end of feature declaration"));
				}
				cur = lexer.next();
				continue;
			}
			if (!cur.isToken(TokIdentifier))
			{
				throw strus::runtime_error( _TXT("feature type name (identifier) expected at start of a feature declaration"));
			}
			std::string identifier = cur.value();
			StatementType statementType = AssignNormalizedTerm;

			cur = lexer.next();
			if (cur.isToken(TokAssign))
			{
				statementType = AssignNormalizedTerm;
			}
			else if (cur.isToken( TokLeftArrow))
			{
				statementType = AssignPatternResult;
			}
			else
			{
				throw strus::runtime_error( _TXT("assignment operator '=' or '<-' expected after set identifier in a feature declaration"));
			}
			cur = lexer.next();
			if (featclass == FeatSubDocument)
			{
				if (statementType == AssignPatternResult) throw strus::runtime_error( "%s", _TXT("pattern result assignment '<-' not allowed in sub document section"));

				std::string xpathexpr( parseSelectorExpression( lexer));
				analyzer->defineSubDocument( identifier, xpathexpr);
			}
			else if (featclass == FeatAggregator)
			{
				if (statementType == AssignPatternResult) throw strus::runtime_error( "%s", _TXT("pattern result assignment '<-' not allowed in aggregator section"));

				strus::local_ptr<AggregatorFunctionInstanceInterface> statfunc;
				FunctionConfig cfg = parseAggregatorFunctionConfig( lexer);

				const AggregatorFunctionInterface* sf = textproc->getAggregator( cfg.name());
				if (!sf) throw strus::runtime_error(_TXT( "unknown aggregator function '%s'"), cfg.name().c_str());
				
				statfunc.reset( sf->createInstance( cfg.args()));
				if (!statfunc.get()) throw strus::runtime_error(_TXT( "failed to create instance of aggregator function '%s'"), cfg.name().c_str());

				analyzer->defineAggregatedMetaData( identifier, statfunc.get());
				statfunc.release();
			}
			else if (featclass == FeatPatternMatch)
			{
				if (statementType == AssignPatternResult) throw strus::runtime_error( "%s", _TXT("pattern result assignment '<-' not allowed in pattern match section"));
				parseAnalyzerPatternMatchProgramDef( lexer, analyzer, textproc, featclassid, identifier, warnings, errorhnd);
			}
			else switch (statementType)
			{
				case AssignPatternResult:
					parseDocumentPatternFeatureDef( lexer, *analyzer, textproc, identifier, featclass);
					break;
				case AssignNormalizedTerm:
					parseDocumentFeatureDef( lexer, *analyzer, textproc, identifier, featclass);
					break;
			}
			if (!lexer.current().isToken( TokSemiColon))
			{
				throw strus::runtime_error( _TXT("semicolon ';' expected at end of feature declaration"));
			}
			cur = lexer.next();
		}
		return true;
	}
	catch (const std::bad_alloc&)
	{
		errorhnd->report( ErrorCodeOutOfMem, _TXT("out of memory parsing document analyzer program"));
		return false;
	}
	catch (const std::runtime_error& e)
	{
		errorhnd->report( ErrorCodeRuntimeError, _TXT("error in document analyzer program on line %d: %s"), lexer.lineno(), e.what());
		return false;
	}
}



