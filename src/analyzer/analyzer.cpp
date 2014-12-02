/*
---------------------------------------------------------------------
    The C++ library strus implements basic operations to build
    a search engine for structured search on unstructured data.

    Copyright (C) 2013,2014 Patrick Frey

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

--------------------------------------------------------------------

	The latest version of strus can be found at 'http://github.com/patrickfrey/strus'
	For documentation see 'http://patrickfrey.github.com/strus'

--------------------------------------------------------------------
*/
#include "analyzer.hpp"
#include "strus/tokenMinerLib.hpp"
#include "strus/tokenMiner.hpp"
#include "strus/tokenMinerFactory.hpp"
#include "strus/normalizerInterface.hpp"
#include "strus/tokenizerInterface.hpp"
#include "parser/lexems.hpp"
#include "textwolf/xmlpathautomatonparse.hpp"
#include "textwolf/xmlpathselect.hpp"
#include "textwolf/charset.hpp"
#include <boost/algorithm/string.hpp>
#include <iostream>
#include <sstream>
#include <set>

using namespace strus;
using namespace strus::parser;

static std::string errorPosition( const char* base, const char* itr)
{
	unsigned int line = 1;
	unsigned int col = 1;
	std::ostringstream msg;

	for (unsigned int ii=0,nn=itr-base; ii < nn; ++ii)
	{
		if (base[ii] == '\n')
		{
			col = 1;
			++line;
		}
		else
		{
			++col;
		}
	}
	msg << "at line " << line << " column " << col;
	return msg.str();
}


class Analyzer::DocumentParser
{
public:
	DocumentParser()
	{}

	enum FeatureClass
	{
		FeatMetaData,
		FeatAttribute,
		FeatTerm
	};
	static const char* featureClassName( FeatureClass i)
	{
		static const char* ar[] = {"MetaData", "Attribute", "Term"};
		return  ar[i];
	}

	void addExpression( const std::string& featurename, const std::string& expression, const TokenMiner* miner, FeatureClass featureClass_)
	{
		m_featuredefar.push_back( FeatureDef( featurename, miner->tokenizer(), miner->normalizer(), featureClass_));

		int featidx = m_featuredefar.size();
		int errorpos = m_automaton.addExpression( featidx, expression.c_str(), expression.size());
		if (errorpos)
		{
			int errorsize = expression.size() - errorpos;
			std::string locstr;
			if (errorsize <= 0)
			{
				locstr = "end of expression";
			}
			else
			{
				if (errorsize > 10) errorsize = 10;
				if (errorpos == 1)
				{
					locstr = "start of expression";
				}
				else
				{
					locstr = std::string("'...") + std::string( expression.c_str() + (errorpos - 1), errorsize) + "'";
				}
			}
			throw std::runtime_error( std::string( "error in selection expression '") + expression + "' at " + locstr);
		}
	}

	class FeatureDef
	{
	public:
		FeatureDef( const std::string& name_,
				const TokenizerInterface* tokenizer_,
				const NormalizerInterface* normalizer_,
				FeatureClass featureClass_)
			:m_name(name_)
			,m_tokenizer(tokenizer_)
			,m_normalizer(normalizer_)
			,m_featureClass(featureClass_){}
	
		const std::string& name() const			{return m_name;}
		const TokenizerInterface* tokenizer() const	{return m_tokenizer;}
		const NormalizerInterface* normalizer() const	{return m_normalizer;}
		FeatureClass featureClass() const		{return m_featureClass;}

		std::vector<tokenizer::Position>
			tokenize( const char* elem, std::size_t elemsize) const
		{
			std::vector<tokenizer::Position> pos;
			if (tokenizer())
			{
				pos = tokenizer()->tokenize( elem, elemsize);
			}
			else
			{
				pos.push_back( tokenizer::Position( 0, elemsize));
			}
			return pos;
		}

	private:
		std::string m_name;
		const TokenizerInterface* m_tokenizer;
		const NormalizerInterface* m_normalizer;
		FeatureClass m_featureClass;
	};

	class Context
	{
	public:
		Context( const DocumentParser& docparser)
		{
			m_normctxsize = docparser.nofFeatureDefs();

			m_normctxar = (NormalizerInterface::Context**)
					std::calloc( m_normctxsize, sizeof( m_normctxar[0]));
			if (!m_normctxar) throw std::bad_alloc();

			for (std::size_t fidx=1; fidx <= m_normctxsize; ++fidx)
			{
				const FeatureDef& fdef = docparser.featureDef( fidx);
				if (fdef.normalizer())
				{
					m_normctxar[ fidx-1] = fdef.normalizer()->createContext();
				}
			}
		}
		~Context()
		{
			if (m_normctxar)
			{
				for (std::size_t fidx=1; fidx <= m_normctxsize; ++fidx)
				{
					if (m_normctxar[ fidx-1]) delete m_normctxar[ fidx-1];
				}
				std::free( m_normctxar);
			}
		}

		NormalizerInterface::Context* normalizerContext( std::size_t featidx)
		{
			if (featidx <= 0 || (std::size_t)featidx > m_normctxsize)
			{
				throw std::runtime_error( "internal: unknown index of feature");
			}
			return m_normctxar[ featidx-1];
		}

	private:
		NormalizerInterface::Context** m_normctxar;
		std::size_t m_normctxsize;
	};

	struct Instance
	{
		typedef textwolf::XMLPathSelectAutomaton<> Automaton;

		Instance( char const* src_, const Automaton* atm_)
			:m_src(src_),m_atm(atm_),m_scanner( src_),m_pathselect(atm_)
		{
			m_itr = m_scanner.begin();
			m_end = m_scanner.end();
			if (m_itr == m_end)
			{
				m_selitr = m_selend = m_pathselect.end();
			}
			else
			{
				m_selitr = m_pathselect.push( m_itr->type(), m_itr->content(), m_itr->size());
				m_selend = m_pathselect.end();
				m_position = m_scanner.getPosition() - m_itr->size();
			}
		}

		typedef textwolf::XMLPathSelect<
				textwolf::charset::UTF8
			> XMLPathSelect;
		typedef textwolf::XMLScanner<
				char const*,
				textwolf::charset::UTF8,
				textwolf::charset::UTF8,
				std::string
			> XMLScanner;

		char const* m_src;
		const Automaton* m_atm;
		XMLScanner m_scanner;
		XMLPathSelect m_pathselect;
		XMLScanner::iterator m_itr;
		XMLScanner::iterator m_end;
		XMLPathSelect::iterator m_selitr;
		XMLPathSelect::iterator m_selend;
		std::size_t m_position;

		bool getNext( const char*& elem, std::size_t& elemsize, int& featidx)
		{
			if (m_itr == m_end) return false;
			while (m_selitr == m_selend)
			{
				++m_itr;
				if (m_itr == m_end) return false;
				if (m_itr->type() == XMLScanner::ErrorOccurred)
				{
					if (m_itr->size())
					{
						throw std::runtime_error( std::string( "error in XML document: ") + std::string(m_itr->content(), m_itr->size()));
					}
					else
					{
						throw std::runtime_error( std::string( "input document is not valid XML"));
					}
				}
				m_selitr = m_pathselect.push( m_itr->type(), m_itr->content(), m_itr->size());
				m_selend = m_pathselect.end();
				m_position = m_scanner.getPosition() - m_itr->size();
			}
			featidx = *m_selitr;
			++m_selitr;
			elem = m_itr->content();
			elemsize = m_itr->size();
			return true;
		}

		std::size_t position() const
		{
			return m_position;
		}
	};

	std::size_t nofFeatureDefs() const
	{
		return m_featuredefar.size();
	}

	const FeatureDef& featureDef( int featidx) const
	{
		if (featidx <= 0 || (std::size_t)featidx > m_featuredefar.size())
		{
			throw std::runtime_error( "internal: unknown index of feature");
		}
		return m_featuredefar[ featidx-1];
	}

	typedef textwolf::XMLPathSelectAutomatonParser<> Automaton;
	const Automaton* automaton() const	
	{
		return &m_automaton;
	}

	void print( std::ostream& out) const
	{
		out << "Automaton:" << std::endl;
		out << m_automaton.tostring() << std::endl;
		out << "Features:" << std::endl;
		std::vector<FeatureDef>::const_iterator fi = m_featuredefar.begin(), fe = m_featuredefar.end();
		for (int fidx=1; fi != fe; ++fi,++fidx)
		{
			out << " " << featureClassName( fi->featureClass());
			out << "[" << fidx << "] " << fi->name() << std::endl;
		}
	}

private:
	Automaton m_automaton;
	std::vector<FeatureDef> m_featuredefar;
};


Analyzer::~Analyzer()
{
	if (m_parser) delete m_parser;
}

static void parseFeatureDef(
	Analyzer::DocumentParser* parser,
	const TokenMinerFactory& tokenMinerFactory,
	const std::string& featurename,
	char const*& src,
	Analyzer::DocumentParser::FeatureClass featureClass)
{
	std::string xpathexpr;
	std::string method;

	if (featureClass != Analyzer::DocumentParser::FeatTerm)
	{
		if (featurename.size() > 1)
		{
			throw std::runtime_error( "using identifier for metadata with length > 1 not allowed (metadata identifiers have to be a single alpha numeric character)");
		}
	}
	if (isAlpha(*src))
	{
		method = parse_IDENTIFIER( src);
	}
	else
	{
		throw std::runtime_error( "identifier (token miner type) expected after assignment operator in a term declaration");
	}
	if (isStringQuote(*src))
	{
		xpathexpr = parse_STRING( src);
	}
	else
	{
		char const* start = src;
		while (*src && !isSpace(*src) && *src != ';') ++src;
		xpathexpr.append( start, src-start);
	}
	const TokenMiner* miner = tokenMinerFactory.get( method);
	if (!miner) throw std::runtime_error( std::string( "unknown token type '") + method + "'");

	parser->addExpression( featurename, xpathexpr, miner, featureClass);
}

Analyzer::Analyzer(
		const TokenMinerFactory& tokenMinerFactory,
		const std::string& source)
	:m_parser(0)
{
	char const* src = source.c_str();
	skipSpaces(src);
	try
	{
		m_parser = new DocumentParser();
		while (*src)
		{
			if (!isAlnum(*src))
			{
				throw std::runtime_error( "alphanumeric identifier (feature set) expected at start of a term declaration");
			}
			std::string name = parse_IDENTIFIER( src);
			if (isAssign( *src))
			{
				parse_OPERATOR(src);
				parseFeatureDef(
					m_parser, tokenMinerFactory, name, src,
					DocumentParser::FeatTerm);
			}
			else if (isColon( *src))
			{
				parse_OPERATOR(src);
				if (isAssign( *src))
				{
					parse_OPERATOR(src);
					parseFeatureDef( 
						m_parser, tokenMinerFactory, name, src,
						DocumentParser::FeatMetaData);
				}
				else
				{
					parseFeatureDef(
						m_parser, tokenMinerFactory, name, src,
						DocumentParser::FeatAttribute);
				}
			}
			else
			{
				throw std::runtime_error( "assignment operator '=' or colon ':' expected after feature set identifier in a term or metadata declaration");
			}
			if (!isSemiColon(*src))
			{
				throw std::runtime_error( "semicolon ';' expected at end of feature declaration");
			}
			parse_OPERATOR(src);
		}
	}
	catch (const std::runtime_error& e)
	{
		throw std::runtime_error(
			std::string( "error in query evaluation program ")
			+ errorPosition( source.c_str(), src)
			+ ":" + e.what());
	}
}

/// \brief Map byte offset positions to token occurrence positions:
static std::vector<analyzer::Term> mapPositions( const std::vector<analyzer::Term>& ar)
{
	std::vector<analyzer::Term> rt;
	std::set<unsigned int> pset;
	std::vector<analyzer::Term>::const_iterator ri = ar.begin(), re = ar.end();
	for (; ri != re; ++ri)
	{
		pset.insert( ri->pos());
	}
	std::map<unsigned int, unsigned int> posmap;
	std::set<unsigned int>::const_iterator pi = pset.begin(), pe = pset.end();
	for (unsigned int pcnt=0; pi != pe; ++pi)
	{
		posmap[ *pi] = ++pcnt;
	}
	for (ri = ar.begin(); ri != re; ++ri)
	{
		unsigned int pos = posmap[ ri->pos()];
		rt.push_back( analyzer::Term( ri->type(), ri->value(), pos));
	}
	return rt;
}


static void normalize(
		analyzer::Document& res,
		NormalizerInterface::Context* normctx,
		const Analyzer::DocumentParser::FeatureDef& feat,
		const char* elem,
		std::size_t elemsize,
		const std::vector<tokenizer::Position>& pos,
		std::size_t curr_position)
{
	std::vector<tokenizer::Position>::const_iterator
		pi = pos.begin(), pe = pos.end();
	for (; pi != pe; ++pi)
	{
		if (feat.normalizer())
		{
			switch (feat.featureClass())
			{
				case Analyzer::DocumentParser::FeatMetaData:
				{
					std::string valstr(
						feat.normalizer()->normalize(
							normctx, elem + pi->pos, pi->size));
					res.addMetaData( feat.name(), valstr);
					break;
				}
				case Analyzer::DocumentParser::FeatAttribute:
				{
					std::string valstr(
						feat.normalizer()->normalize(
							normctx, elem + pi->pos, pi->size));
					res.addAttribute( feat.name(), valstr);
					break;
				}
				case Analyzer::DocumentParser::FeatTerm:
				{
					std::string valstr(
						feat.normalizer()->normalize(
							normctx, elem + pi->pos, pi->size));
					res.addTerm(
						feat.name(), valstr,
						curr_position + pi->pos);
					break;
				}
			}
		}
		else
		{
			switch (feat.featureClass())
			{
				case Analyzer::DocumentParser::FeatMetaData:
				{
					res.addMetaData(
						feat.name(),
						std::string( elem + pi->pos, pi->size));
					break;
				}
				case Analyzer::DocumentParser::FeatAttribute:
					res.addAttribute(
						feat.name(),
						std::string( elem + pi->pos, pi->size));
					break;
				case Analyzer::DocumentParser::FeatTerm:
					res.addTerm(
						feat.name(),
						std::string( elem + pi->pos, pi->size),
						curr_position + pi->pos);
					break;
			}
		}
	}
}


struct Chunk
{
	Chunk()
		:position(0){}
	Chunk( std::size_t position_, const std::string& content_)
		:position(position_),content(content_){}
	Chunk( const Chunk& o)
		:position(o.position),content(o.content){}

	std::size_t position;
	std::string content;
};

analyzer::Document Analyzer::analyze( const std::string& content) const
{
	analyzer::Document rt;
	DocumentParser::Instance scanner( content.c_str(), m_parser->automaton());
	const char* elem = 0;
	std::size_t elemsize = 0;
	int featidx = 0;
	std::size_t curr_position = 0;
	typedef std::map<int,Chunk> ConcatenatedMap;
	ConcatenatedMap concatenatedMap;
	DocumentParser::Context ctx( *m_parser);

	// [1] Scan the document and push the normalized tokenization of the elements to the result:
	while (scanner.getNext( elem, elemsize, featidx))
	{
		try
		{
			curr_position = scanner.position();
			const Analyzer::DocumentParser::FeatureDef& feat
				= m_parser->featureDef( featidx);
			NormalizerInterface::Context* normctx
				= ctx.normalizerContext( featidx);
	
			if (feat.tokenizer() && feat.tokenizer()->concatBeforeTokenize())
			{
				ConcatenatedMap::iterator ci
					= concatenatedMap.find( featidx);
				if (ci == concatenatedMap.end())
				{
					concatenatedMap[ featidx]
						= Chunk( curr_position,
							 std::string( elem, elemsize));
				}
				else
				{
					Chunk& cm = concatenatedMap[ featidx];
					std::size_t newlen
						= curr_position - cm.position;
					cm.content.resize( newlen, ' ');
					cm.content.append( elem, elemsize);
				}
				continue;
			}
			std::vector<tokenizer::Position>
				pos = feat.tokenize( elem, elemsize);
			
			normalize( rt, normctx, feat, elem, elemsize, pos, curr_position);
		}
		catch (const std::runtime_error& err)
		{
			throw std::runtime_error( std::string( "error in analyze when processing chunk (") + std::string( elem, elemsize) + "): " + err.what());
		}
	}
	ConcatenatedMap::const_iterator
		ci = concatenatedMap.begin(),
		ce = concatenatedMap.end();

	for (; ci != ce; ++ci)
	{
		curr_position = ci->second.position;
		const Analyzer::DocumentParser::FeatureDef& feat
			= m_parser->featureDef( ci->first);
		NormalizerInterface::Context* normctx
			= ctx.normalizerContext( featidx);

		std::vector<tokenizer::Position>
			pos = feat.tokenize(
				ci->second.content.c_str(),
				ci->second.content.size());

		normalize( rt, normctx, feat, ci->second.content.c_str(),
				ci->second.content.size(), pos, curr_position);
	}
	return analyzer::Document(
			rt.metadata(),
			rt.attributes(),
			mapPositions( rt.terms())); 
}

void Analyzer::print( std::ostream& out) const
{
	m_parser->print( out);
}

