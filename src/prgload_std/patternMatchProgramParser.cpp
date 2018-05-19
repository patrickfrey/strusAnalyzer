/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Implements the parser for a pattern match program
/// \file patternMatchProgramParser.hpp
#include "patternMatchProgramParser.hpp"
#include "strus/base/programLexer.hpp"
#include "strus/errorBufferInterface.hpp"
#include "strus/debugTraceInterface.hpp"
#include "strus/patternMatcherInterface.hpp"
#include "strus/patternLexerInterface.hpp"
#include "strus/patternTermFeederInterface.hpp"
#include "strus/base/string_format.hpp"
#include "strus/base/string_conv.hpp"
#include "strus/base/numstring.hpp"
#include "strus/base/utf8.hpp"
#include "private/internationalization.hpp"
#include <iostream>
#include <sstream>
#include <limits>

#define STRUS_DBGTRACE_COMPONENT_NAME "pattern"
using namespace strus;

PatternMatcherProgramParser::PatternMatcherProgramParser(
		PatternLexerInstanceInterface* crm,
		PatternMatcherInstanceInterface* tpm,
		ErrorBufferInterface* errorhnd_)
	:m_errorhnd(errorhnd_)
	,m_debugtrace(0)
	,m_patternMatcher(tpm)
	,m_patternLexer(crm)
	,m_patternTermFeeder(0)
	,m_regexNameSymbolTab()
	,m_patternNameSymbolTab()
	,m_lexemSymbolTab()
	,m_patternLengthMap()
	,m_symbolRegexIdList()
	,m_unresolvedPatternNameSet()
{
	if (!m_patternMatcher || !m_patternLexer) throw strus::runtime_error( "failed to create pattern matching structures to instrument");
	DebugTraceInterface* dbg = m_errorhnd->debugTrace();
	m_debugtrace = dbg ? dbg->createTraceContext( STRUS_DBGTRACE_COMPONENT_NAME) : NULL;
}

PatternMatcherProgramParser::PatternMatcherProgramParser(
		PatternTermFeederInstanceInterface* tfm,
		PatternMatcherInstanceInterface* tpm,
		ErrorBufferInterface* errorhnd_)
	:m_errorhnd(errorhnd_)
	,m_patternMatcher(tpm)
	,m_patternLexer(0)
	,m_patternTermFeeder(tfm)
	,m_regexNameSymbolTab()
	,m_patternNameSymbolTab()
	,m_lexemSymbolTab()
	,m_symbolRegexIdList()
	,m_unresolvedPatternNameSet()
{
	if (!m_patternMatcher || !m_patternTermFeeder) throw strus::runtime_error( "failed to create pattern matching structures to instrument");
}

PatternMatcherProgramParser::~PatternMatcherProgramParser()
{
	if (m_debugtrace) delete m_debugtrace;
}

enum Tokens {
	TokLEXER,
	TokMATCHER,
	TokFEEDER,
	TokIdentifier,
	TokFloat,
	TokInteger,
	TokOpenOvalBracket,
	TokCloseOvalBracket,
	TokOpenSquareBracket,
	TokCloseSquareBracket,
	TokOr,
	TokPercent,
	TokAssign,
	TokComma,
	TokColon,
	TokSemiColon,
	TokDot,
	TokExp,
	TokEditDistance,
	TokLeftArrow,
	TokRightArrow
};
static const char* g_tokens[] = {
	"\\%LEXER",
	"\\%MATCHER",
	"\\%FEEDER",
	"[a-zA-Z_][a-zA-Z0-9_]*",
	"[+-]*[0-9][0-9_]*[.][0-9]*",
	"[+-]*[0-9][0-9_]*",
	"\\(",
	"\\)",
	"\\[",
	"\\]",
	"\\|",
	"\\%",
	"\\=",
	",",
	":",
	";",
	"\\.",
	"\\^",
	"~[0-9][0-9_]*",
	"<-",
	"->",
	NULL
};
static const char* g_errtokens[] = {
	"[0-9][0-9]*[a-zA-Z_]",
	NULL
};
static const char* g_eolncomment = "#";

static bool isEqual( const std::string& id, const char* idstr)
{
	char const* si = id.c_str();
	char const* di = idstr;
	for (; *si && *di && ((*si|32) == (*di|32)); ++si,++di){}
	return !*si && !*di;
}

static std::string parse_REGEX( char const*& src)
{
	std::string rt;
	char eb = *src++;
	while (*src != eb)
	{
		if (*src == '\0' || *src == '\n' || *src == '\r') throw strus::runtime_error(_TXT("unterminated regular expression %c...%c"), eb, eb);
		if (*src == '\\')
		{
			rt.push_back( *src++);
			if (*src == '\0' || *src == '\n' || *src == '\r') throw strus::runtime_error(_TXT("unterminated regular expression %c...%c"), eb, eb);
		}
		rt.push_back( *src++);
	}
	++src;
	return rt;
}

void PatternMatcherProgramParser::loadMatcherOption( ProgramLexer& lexer)
{
	if (lexer.current().isToken( TokIdentifier))
	{
		std::string name = lexer.current().value();
		if (lexer.next().isToken( TokAssign))
		{
			lexer.next();
			if (lexer.current().isToken( TokFloat) || lexer.current().isToken( TokInteger))
			{
				double value = numstring_conv::todouble( lexer.current().value());
				m_patternMatcher->defineOption( name, value);
			}
			else
			{
				throw strus::runtime_error( _TXT("expected number as matcher option value after assign"));
			}
		}
		else
		{
			m_patternMatcher->defineOption( name, 0.0);
		}
	}
	else
	{
		throw strus::runtime_error( _TXT("identifier expected at start of pattern matcher option declaration"));
	}
}

void PatternMatcherProgramParser::loadLexerOption( ProgramLexer& lexer)
{
	if (lexer.current().isToken( TokIdentifier))
	{
		std::string name = lexer.current().value();
		lexer.next();
		m_patternLexer->defineOption( name, 0);
	}
	else
	{
		throw strus::runtime_error( _TXT("identifier expected at start of pattern lexer option declaration"));
	}
}

void PatternMatcherProgramParser::loadFeederOption( ProgramLexer& lexer)
{
	if (lexer.current().isToken( TokIdentifier))
	{
		std::string name = lexer.current().value();
		if (isEqual( name, "lexem"))
		{
			lexer.next();
			if (!lexer.current().isToken( TokIdentifier))
			{
				throw strus::runtime_error( _TXT("identifier expected as argument of feeder option 'lexem'"));
			}
			std::string lexemid( lexer.current().value());
			(void)defineAnalyzerTermType( lexemid);
			lexer.next();
		}
		else
		{
			throw strus::runtime_error(_TXT("unknown feeder option '%s'"), name.c_str());
		}
	}
	else
	{
		throw strus::runtime_error( _TXT("option name expected at start of pattern feeder option declaration"));
	}
}

void PatternMatcherProgramParser::loadOptions( ProgramLexer& lexer)
{
	while (!lexer.current().end())
	{
		if (lexer.current().isToken( TokLEXER) || lexer.current().isToken( TokMATCHER) || lexer.current().isToken( TokFEEDER))
		{
			if (lexer.current().isToken( TokLEXER))
			{
				if (!m_patternLexer)
				{
					throw strus::runtime_error( _TXT("defined '%%LEXER' option without lexer defined"));
				}
				do
				{
					lexer.next();
					loadLexerOption( lexer);
				} while (lexer.current().isToken(TokComma));
			}
			else if (lexer.current().isToken( TokMATCHER))
			{
				do
				{
					lexer.next();
					loadMatcherOption( lexer);
				} while (lexer.current().isToken(TokComma));
			}
			else //if (lexer.current().isToken( TokFEEDER))
			{
				if (!m_patternTermFeeder)
				{
					throw strus::runtime_error( _TXT("defined '%%FEEDER' option without feeder defined"));
				}
				do
				{
					lexer.next();
					loadFeederOption( lexer);
				} while (lexer.current().isToken(TokComma));
			}
		}
		else
		{
			break;
		}
	}
}

void PatternMatcherProgramParser::loadLexerRule( ProgramLexer& lexer, const std::string& name, unsigned int level)
{
	if (m_patternLexer)
	{
		unsigned int nameid = m_regexNameSymbolTab.getOrCreate( name);
		if (nameid == 0)
		{
			throw strus::runtime_error( _TXT("failed to define lexem name symbol"));
		}
		if (nameid >= MaxPatternTermNameId)
		{
			throw strus::runtime_error(_TXT("too many regular expression tokens defined: %u"), nameid);
		}
		if (m_regexNameSymbolTab.isNew())
		{
			m_patternLexer->defineLexemName( nameid, name);
		}
		std::string regex;
		do
		{
			char const* rxptr = lexer.nextpos();
			regex = parse_REGEX( rxptr);
			lexer.skipto( rxptr);

			if (lexer.next().isToken(TokEditDistance))
			{
				//... edit distance operator "~1","~2",....
				regex.append( lexer.current().value());
				lexer.next();
			}
			unsigned int resultIndex = 0;
			if (lexer.current().isToken(TokOpenSquareBracket))
			{
				if (!lexer.next().isToken( TokInteger))
				{
					throw strus::runtime_error( _TXT("expected result index (unsigned inside square brackets '[' .. ']'"));
				}
				resultIndex = numstring_conv::touint( lexer.current().value(), std::numeric_limits<char>::max());
				if (!lexer.next().isToken( TokCloseSquareBracket))
				{
					throw strus::runtime_error( _TXT("close square bracket ']' expected at end of result index definition"));
				}
				lexer.next();
			}
			analyzer::PositionBind posbind = analyzer::BindContent;
			if (lexer.current().isToken( TokLeftArrow))
			{
				lexer.next();
				posbind = analyzer::BindPredecessor;
			}
			else if (lexer.current().isToken( TokRightArrow))
			{
				lexer.next();
				posbind = analyzer::BindSuccessor;
			}
			m_patternLexer->defineLexem( nameid, regex, resultIndex, level, posbind);
		}
		while (lexer.current().isToken( TokOr));
	}
	else if (m_patternTermFeeder)
	{
		throw strus::runtime_error( _TXT("pattern analyzer terms are defined by option %%lexem type and not with id : regex"));
	}
	else
	{
		throw strus::runtime_error( _TXT("loading lexer rule without lexer defined"));
	}
}

void PatternMatcherProgramParser::loadMatcherRule( ProgramLexer& lexer, const std::string& name, bool visible)
{
	ProgramLexem cur;
	//... token pattern expression declaration
	unsigned int nameid = m_patternNameSymbolTab.getOrCreate( name);
	if (nameid == 0)
	{
		throw strus::runtime_error( _TXT("failed to define pattern name symbol"));
	}
	do
	{
		//... Token pattern def -> name = expression ;
		cur = lexer.next();
		SubExpressionInfo exprinfo;
		loadExpression( lexer, exprinfo);
		std::set<uint32_t>::iterator ui = m_unresolvedPatternNameSet.find( nameid);
		if (ui != m_unresolvedPatternNameSet.end())
		{
			m_unresolvedPatternNameSet.erase( ui);
		}
		std::map<uint32_t,unsigned int>::iterator li = m_patternLengthMap.find( nameid);
		if (li != m_patternLengthMap.end())
		{
			li->second = std::max( li->second, exprinfo.maxrange);
		}
		else
		{
			m_patternLengthMap[ nameid] = exprinfo.maxrange;
		}
		m_patternMatcher->definePattern( name, visible);
	}
	while (lexer.current().isToken( TokOr));
}

void PatternMatcherProgramParser::loadRules( ProgramLexer& lexer)
{
	ProgramLexem cur;
	while (!lexer.current().end())
	{
		bool visible = true;
		if (lexer.current().isToken( TokDot))
		{
			//... declare rule as invisible (private)
			visible = false;
			cur = lexer.next();
		}
		if (lexer.current().isToken( TokIdentifier) || lexer.current().isString())
		{
			bool nameIsString = lexer.current().isString();
			std::string name = lexer.current().value();
			if (name.empty()) throw strus::runtime_error( _TXT("pattern name is empty"));
			lexer.next();

			int level = -1;
			if (lexer.current().isToken( TokExp))
			{
				lexer.next();
				if (!lexer.current().isToken( TokInteger)) throw strus::runtime_error( _TXT("expected unsigned integer as level"));
				level = (int)(unsigned int)numstring_conv::touint( lexer.current().value(), std::numeric_limits<int>::max());
				cur = lexer.next();
			}
			if (lexer.current().isToken( TokColon))
			{
				//... lexem expression declaration
				if (nameIsString) throw strus::runtime_error( _TXT("string not allowed as lexem type"));
				if (!visible) throw strus::runtime_error( _TXT("unexpected colon ':' after dot '.' followed by an identifier, that starts an token pattern declaration marked as private (invisible in output)"));

				loadLexerRule( lexer, name, level>=0?level:0);
			}
			else if (lexer.current().isToken( TokAssign))
			{
				if (level >= 0) throw strus::runtime_error( _TXT("unsupported definition of level \"^N\" in token pattern definition"));

				loadMatcherRule( lexer, name, visible);
			}
			else
			{
				throw strus::runtime_error( _TXT("assign '=' (token pattern definition) or colon ':' (lexem pattern definition) expected after name starting a pattern declaration"));
			}
			if (!lexer.current().isToken( TokSemiColon))
			{
				throw strus::runtime_error( _TXT("semicolon ';' expected at end of rule"));
			}
			lexer.next();
			if (m_errorhnd->hasError())
			{
				throw strus::runtime_error( _TXT("error in rule definition"));
			}
		}
		else
		{
			throw strus::runtime_error( _TXT("identifier or string expected at start of rule"));
		}
	}
}

bool PatternMatcherProgramParser::load( const std::string& source)
{
	ProgramLexer lexer( source.c_str(), g_eolncomment, g_tokens, g_errtokens, m_errorhnd);
	if (m_errorhnd->hasError()) return false;
	try
	{
		lexer.next();
		loadOptions( lexer);
		loadRules( lexer);

		return true;
	}
	catch (const std::runtime_error& err)
	{
		std::string locinfo = lexer.currentLocationString( -20, 50, "<!>");
		int lineno = lexer.lineno();
		m_errorhnd->report( ErrorCodeRuntimeError, _TXT("error in pattern match program line %d: '%s' [at '%s']"), lineno, err.what(), locinfo.c_str());
		return false;
	}
	catch (const std::bad_alloc&)
	{
		m_errorhnd->report( ErrorCodeOutOfMem, _TXT("out of memory when loading program source"));
		return false;
	}
}

bool PatternMatcherProgramParser::compile()
{
	try
	{
		if (m_errorhnd->hasError())
		{
			m_errorhnd->explain( _TXT("error before compile (while building program): %s"));
			return false;
		}
		if (!m_unresolvedPatternNameSet.empty())
		{
			std::ostringstream unresolved;
			std::set<uint32_t>::iterator
				ui = m_unresolvedPatternNameSet.begin(),
				ue = m_unresolvedPatternNameSet.end();
			for (std::size_t uidx=0; ui != ue && uidx<10; ++ui,++uidx)
			{
				if (m_debugtrace) m_debugtrace->event( "unresolved", "%s", m_patternNameSymbolTab.key(*ui));
			}
		}
		bool rt = true;
		rt &= m_patternMatcher->compile();
		if (m_patternLexer)
		{
			rt &= m_patternLexer->compile();
		}
		return rt;
	}
	catch (const std::runtime_error& e)
	{
		m_errorhnd->report( ErrorCodeRuntimeError, _TXT("failed to compile pattern match program source: %s"), e.what());
		return false;
	}
	catch (const std::bad_alloc&)
	{
		m_errorhnd->report( ErrorCodeOutOfMem, _TXT("out of memory when compiling pattern match program source"));
		return false;
	}
}

typedef strus::PatternMatcherInstanceInterface::JoinOperation JoinOperation;
static JoinOperation joinOperation( const std::string& name)
{
	JoinOperation rt = (JoinOperation)0;
	const char* opname;
	for (; !!(opname=strus::PatternMatcherInstanceInterface::joinOperationName(rt)); rt = (JoinOperation)((int)rt+1))
	{
		if (strus::caseInsensitiveEquals( name, opname))
		{
			return rt;
		}
	}
	throw strus::runtime_error( _TXT("unknown join operation: '%s'"), name.c_str());
}

uint32_t PatternMatcherProgramParser::getOrCreateSymbol( unsigned int regexid, const std::string& name)
{
	char regexidbuf[ 16];
	std::size_t regexidsize = utf8encode( regexidbuf, regexid+1);
	std::string symkey( regexidbuf, regexidsize);
	symkey.append( name);
	uint32_t symid = m_lexemSymbolTab.getOrCreate( symkey) + MaxPatternTermNameId;
	if (m_lexemSymbolTab.isNew())
	{
		m_symbolRegexIdList.push_back( regexid);
		if ((std::size_t)( symid - MaxPatternTermNameId) != m_symbolRegexIdList.size())
		{
			throw strus::runtime_error( _TXT("internal: inconsisteny in lexem symbol map"));
		}
		if (m_patternLexer)
		{
			if (m_debugtrace) m_debugtrace->event( "lexer-symbol", "%d %s", regexid, name.c_str());
			m_patternLexer->defineSymbol( symid, regexid, name);
			m_patternLexer->defineLexemName( symid, name);
		}
		else if (m_patternTermFeeder)
		{
			if (m_debugtrace) m_debugtrace->event( "feeder-symbol", "%d %s", regexid, name.c_str());
			m_patternTermFeeder->defineSymbol( symid, regexid, name);
		}
		else
		{
			throw strus::runtime_error( _TXT("internal: no lexer or term feeder defined"));
		}
	}
	return symid;
}

const char* PatternMatcherProgramParser::getSymbolRegexId( unsigned int id) const
{
	const char* symkey = m_lexemSymbolTab.key( id);
	unsigned int symkeyhdrlen = utf8charlen( *symkey);
	if (!symkeyhdrlen) throw strus::runtime_error( _TXT("illegal key in pattern lexem symbol table"));
	int regexid = utf8decode( symkey, symkeyhdrlen) - 1;
	return m_regexNameSymbolTab.key( regexid);
}

unsigned int PatternMatcherProgramParser::defineAnalyzerTermType( const std::string& type)
{
	unsigned int typid = m_regexNameSymbolTab.getOrCreate( type);
	if (typid == 0)
	{
		throw strus::runtime_error( _TXT("failed to define term type symbol"));
	}
	if (typid >= MaxPatternTermNameId)
	{
		throw strus::runtime_error(_TXT("too many term types defined: %u"), typid);
	}
	if (m_regexNameSymbolTab.isNew())
	{
		m_patternTermFeeder->defineLexem( typid, type);
	}
	return typid;
}

unsigned int PatternMatcherProgramParser::getAnalyzerTermType( const std::string& type) const
{
	return m_regexNameSymbolTab.get( type);
}

void PatternMatcherProgramParser::loadExpressionNode( ProgramLexer& lexer, const std::string& name, SubExpressionInfo& exprinfo)
{
	exprinfo.minrange = 0;
	exprinfo.maxrange = 0;
	if (lexer.current().isToken( TokOpenOvalBracket))
	{
		JoinOperation operation = joinOperation( name);

		unsigned int cardinality = 0;
		unsigned int range = 0;
		unsigned int nofArguments = 0;

		if (!lexer.next().isToken(TokCloseOvalBracket))
		do
		{
			SubExpressionInfo argexprinfo;
			loadExpression( lexer, argexprinfo);
			switch (operation)
			{
				case PatternMatcherInstanceInterface::OpSequence:
					exprinfo.minrange += argexprinfo.minrange;
					exprinfo.maxrange += argexprinfo.maxrange;
					break;
				case PatternMatcherInstanceInterface::OpSequenceImm:
					exprinfo.minrange += argexprinfo.minrange;
					exprinfo.maxrange += argexprinfo.maxrange;
					break;
				case PatternMatcherInstanceInterface::OpSequenceStruct:
					if (nofArguments)
					{
						exprinfo.minrange += argexprinfo.minrange;
						exprinfo.maxrange += argexprinfo.maxrange;
					}
					break;
				case PatternMatcherInstanceInterface::OpWithin:
					exprinfo.minrange += argexprinfo.minrange;
					exprinfo.maxrange += argexprinfo.maxrange;
					break;
				case PatternMatcherInstanceInterface::OpWithinStruct:
					if (nofArguments)
					{
						exprinfo.minrange += argexprinfo.minrange;
						exprinfo.maxrange += argexprinfo.maxrange;
					}
					break;
				case PatternMatcherInstanceInterface::OpAny:
					if (nofArguments == 0 || exprinfo.minrange < argexprinfo.minrange)
					{
						exprinfo.minrange = argexprinfo.minrange;
					}
					if (nofArguments == 0 || exprinfo.maxrange < argexprinfo.maxrange)
					{
						exprinfo.maxrange = argexprinfo.maxrange;
					}
					break;
				case PatternMatcherInstanceInterface::OpAnd:
					if (nofArguments == 0 || exprinfo.minrange > argexprinfo.minrange)
					{
						exprinfo.minrange = argexprinfo.minrange;
					}
					if (nofArguments == 0 || exprinfo.maxrange < argexprinfo.maxrange)
					{
						exprinfo.maxrange = argexprinfo.maxrange;
					}
					break;
			}
			++nofArguments;
			if (lexer.current().isToken(TokOr) || lexer.current().isToken(TokExp))
			{
				unsigned int mask = 0;
				while (lexer.current().isToken(TokOr) || lexer.current().isToken(TokExp))
				{
					if (lexer.current().isToken(TokOr))
					{
						if ((mask & 0x01) == 0)
						{
							mask |= 0x01;
							if (!lexer.next().isToken(TokInteger))
							{
								throw strus::runtime_error( _TXT("unsigned integer expected as proximity range value after '|' in expression parameter list"));
							}
							range = numstring_conv::touint( lexer.current().value(), std::numeric_limits<int>::max());
							lexer.next();
						}
						else
						{
							throw strus::runtime_error( _TXT("duplicate definition of range"));
						}
					}
					else if (lexer.current().isToken(TokExp))
					{
						if ((mask & 0x02) == 0)
						{
							mask |= 0x02;
							if (!lexer.next().isToken(TokInteger))
							{
								throw strus::runtime_error( _TXT("unsigned integer expected as cardinality value after '^' in expression parameter list"));
							}
							cardinality = numstring_conv::touint( lexer.current().value(), std::numeric_limits<short>::max());
							lexer.next();
						}
						else
						{
							throw strus::runtime_error( _TXT("duplicate definition of cardinality"));
						}
					}
				}
				if (lexer.current().isToken(TokComma))
				{
					throw strus::runtime_error( _TXT("unexpected comma ',' after proximity range and/or cardinality specification than must only appear at the end of the arguments list"));
				}
			}
		}
		while (lexer.current().isToken(TokComma) && lexer.next());
		if (!lexer.current().isToken(TokCloseOvalBracket))
		{
			throw strus::runtime_error( _TXT("close bracket ')' expected at end of join operation expression"));
		}
		lexer.next();
		if (range == 0 && exprinfo.maxrange == 0)
		{
			throw strus::runtime_error( _TXT("cannot evaluate length of expression, range has to be specified here"));
		}
		switch (operation)
		{
			case PatternMatcherInstanceInterface::OpSequenceImm:
				if (range == 0)
				{
					range = exprinfo.minrange;
				}
				else if (range < exprinfo.minrange)
				{
					throw strus::runtime_error(_TXT("rule cannot match in such a within such a small position range span: %u (required %u)"), range, exprinfo.minrange);
				}
				break;
			case PatternMatcherInstanceInterface::OpSequence:
			case PatternMatcherInstanceInterface::OpSequenceStruct:
			case PatternMatcherInstanceInterface::OpWithin:
			case PatternMatcherInstanceInterface::OpWithinStruct:
			case PatternMatcherInstanceInterface::OpAny:
			case PatternMatcherInstanceInterface::OpAnd:
				if (range == 0)
				{
					range = exprinfo.maxrange;
				}
				else if (range < exprinfo.minrange)
				{
					throw strus::runtime_error(_TXT("rule cannot match in such a small position range span specified: %u (required %u)"), range, exprinfo.minrange);
				}
				break;
		}
		if (m_debugtrace) m_debugtrace->event( "expression", "op %s card %d range %d args %d", name.c_str(), cardinality, range, nofArguments);
		m_patternMatcher->pushExpression( operation, nofArguments, range, cardinality);
	}
	else if (lexer.current().isToken( TokAssign))
	{
		throw strus::runtime_error( _TXT("unexpected assignment operator '=', only one assignment allowed per node"));
	}
	else
	{
		unsigned int id;
		if (lexer.current().isString())
		{
			if (m_patternLexer)
			{
				id = m_regexNameSymbolTab.get( name);
				if (!id) throw strus::runtime_error(_TXT("undefined lexem '%s'"), name.c_str());
			}
			else
			{
				id = defineAnalyzerTermType( name);
			}
			id = getOrCreateSymbol( id, lexer.current().value());
			if (id)
			{
				m_patternMatcher->pushTerm( id);
				exprinfo.minrange = 1;
				exprinfo.maxrange = 1;
			}
			lexer.next();
		}
		else
		{
			if (m_patternLexer)
			{
				id = m_regexNameSymbolTab.get( name);
			}
			else
			{
				id = getAnalyzerTermType( name);
			}
			if (id)
			{
				m_patternMatcher->pushTerm( id);
				exprinfo.minrange = 1;
				exprinfo.maxrange = 1;
			}
			else
			{
				id = m_patternNameSymbolTab.get( name);
				if (!id)
				{
					id = m_patternNameSymbolTab.getOrCreate( name);
					if (id == 0)
					{
						throw strus::runtime_error( _TXT("failed to define pattern name symbol"));
					}
					m_unresolvedPatternNameSet.insert( id);
					exprinfo.minrange = 0;
					exprinfo.maxrange = 0;
				}
				else if (m_unresolvedPatternNameSet.find( id) != m_unresolvedPatternNameSet.end())
				{
					exprinfo.minrange = 0;
					exprinfo.maxrange = 0;
				}
				else
				{
					std::map<uint32_t,unsigned int>::const_iterator li = m_patternLengthMap.find( id);
					if (li == m_patternLengthMap.end())
					{
						throw strus::runtime_error( _TXT("cannot evaluate length of pattern"));
					}
					exprinfo.minrange = li->second;
					exprinfo.maxrange = li->second;
				}
				m_patternMatcher->pushPattern( name);
			}
		}
	}
}

void PatternMatcherProgramParser::loadExpression( ProgramLexer& lexer, SubExpressionInfo& exprinfo)
{
	if (!lexer.current().isToken( TokIdentifier))
	{
		throw strus::runtime_error( _TXT("identifier expected as name of expression"));
	}
	std::string name = lexer.current().value();
	if (name.empty())
	{
		throw strus::runtime_error( _TXT("name in expression is empty"));
	}
	if (lexer.next().isToken(TokAssign))
	{
		if (m_debugtrace) m_debugtrace->event( "assign", "%s", name.c_str());
		if (!lexer.next().isToken(TokIdentifier))
		{
			throw strus::runtime_error( _TXT("expected variable after assign '='"));
		}
		std::string op = lexer.current().value();
		lexer.next();
		loadExpressionNode( lexer, op, exprinfo);
		std::string formatstring;
		if (lexer.next().isToken(TokOpenSquareBracket))
		{
			lexer.next();
			if (lexer.current().isString())
			{
				formatstring = lexer.current().value();
				if (lexer.next().isToken(TokCloseSquareBracket))
				{
					lexer.next();
				}
				else
				{
					throw strus::runtime_error( _TXT("expected close square brackets ']' after format string"));
				}
			}
			else
			{
				throw strus::runtime_error( _TXT("expected string (format string) in square brackets '[' ']' after expression"));
			}
		}
		m_patternMatcher->attachVariable( name, formatstring);
	}
	else
	{
		loadExpressionNode( lexer, name, exprinfo);
	}
}

