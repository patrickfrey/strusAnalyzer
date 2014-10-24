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
#ifndef _STRUS_ANALYZER_HPP_INCLUDED
#define _STRUS_ANALYZER_HPP_INCLUDED
#include "analyzer.hpp"
#include "parser/lexems.hpp"
#include "textwolf/xmlpathautomatonparse.hpp"
#include <boost/algorithm/string.hpp>
#include <iostream>
#include <sstream>

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

class Tokenizer
{
public:
	virtual ~Tokenizer(){}
	struct Position
	{
		unsigned int pos;
		unsigned int size;

		Position( unsigned int pos_, unsigned int size_)
			:pos(pos_),size(size_){}
		Position( const Position& o)
			:pos(o.pos),size(o.size){}
	};

	virtual std::vector<Position> tokenize( const char* src, std::size_t srcsize) const=0;
};

class Normalizer
{
public:
	virtual ~Normalizer(){}
	virtual std::string normalize( const char* src, std::size_t srcsize) const=0;
};


class Analyzer::DocumentParser
{
public:
	DocumentParser(){}

	void addExpression( const std::string& featurename, const std::string& expression, const Tokenizer* tokenizer, const Normalizer* normalizer)
	{
		int featidx = createFeatureDef( featurename, tokenizer, normalizer);
		m_automaton.addExpression( featidx, expression.c_str(), expression.size());
	}

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
				charset::UTF8
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

		bool getNext( const char*& elem, const std::size_t& elemsize, int& featidx)
		{
			if (m_itr == m_end) return false;
			while (m_selitr == m_selend)
			{
				++m_itr;
				if (m_itr == m_end) return false;
				m_selitr = m_pathselect.push( m_itr->type(), m_itr->content(), m_itr->size());
				m_selend = m_pathselect.end();
				m_position = m_scanner.getPosition() - m_itr->size();
			}
			featidx = *m_selitr;
			elem = m_itr->content();
			elemsize = m_itr->size();
			return true;
		}

		std::size_t position() const
		{
			return m_position;
		}
	};

	class FeatureDef
	{
	public:
		FeatureDef( const std::string& name_,
				const Tokenizer* tokenizer_,
				const Normalizer* normalizer_)
			:m_name(name_),m_tokenizer(tokenizer_),m_normalizer(normalizer_){}

		const std::string& name() const		{return m_name;}
		const Tokenizer* tokenizer() const	{return m_tokenizer;}
		const Normalizer* normalizer() const	{return m_normalizer;}

	private:
		std::string m_name;
		const Tokenizer* m_tokenizer;
		const Normalizer* m_normalizer;
	};

	const FeatureDef& featureDef( int featidx) const
	{
		if (featidx <= 0 || featidx > m_featuredefar.size())
		{
			throw std::runtime_error( "internal: unknown index of feature");
		}
		return m_featuredefar[ featidx-1];
	}

	int createFeatureDef( const std::string& name_,
				const Tokenizer* tokenizer_,
				const Normalizer* normalizer_)
	{
		m_featuredefar.push_back( FeatureDef( name_, tokenizer_, normalizer_));
		return m_featuredefar.size();
	}

	const Automaton* automaton() const	{return m_automaton;}

private:
	typedef textwolf::XMLPathSelectAutomatonParser<> Automaton;
	Automaton m_automaton;
	std::vector<FeatureDef> m_featuredefar;
};


Analyzer::~Analyzer()
{
	if (m_parser) delete m_parser;
}

Analyzer::Analyzer( const std::string& source)
	:m_parser(0)
{
	char const* src = source.c_str();
	skipSpaces(src);
	try
	{
		m_parser = new DocumentParser();
		while (*src)
		{
			if (!isAlpha(*src))
			{
				throw std::runtime_error( "identifier (feature set) expected at start of a term declaration");
			}
			std::string featurename = parse_IDENTIFIER( src);
			std::string xpathexpr;
			std::string method;

			if (!isAssign( *src))
			{
				throw std::runtime_error( "assign '=' expected after feature set identifier in a term declaration");
			}
			parse_OPERATOR(src);
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
			if (isAlpha(*src))
			{
				method = parse_IDENTIFIER( src);
			}
			if (!isSemiColon(*src))
			{
				throw std::runtime_error( "semicolon '=' expected at end of feature declaration");
			}
			parse_OPERATOR(src);

			m_parser->addExpression( featurename, xpathexpr);
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


std::vector<AnalyzerInterface::Term> Analyzer::analyze( const std::string& content) const
{
	std::vector<Term> rt;
	DocumentParser::Instance scanner( content.c_str(), m_parser->automaton());
	const char* elem = 0;
	const std::size_t elemsize = 0;
	int featidx = 0;

	// [1] Scan the document and push the normalized tokenization of the elements to the result:
	while (scanner.getNext( elem, elemsize, featidx))
	{
		const Analyzer::DocumentParser::FeatureDef& feat
			= m_parser->featureDef( featidx);

		std::vector<Tokenizer::Position> pos
			= feat.tokenizer()->tokenize( elem, elemsize);
		std::vector<Tokenizer::Position>::const_iterator pi = pos.begin(), pe = pos.end();

		for (; pi != pe; ++pi)
		{
			rt.push_back(
				Term( feat.name(),
					feat.normalizer()->normalize( elem + pi->pos, pi->size),
					scanner.position()));
		}
	}

	// [2] Map byte offset positions to token occurrence positions:
	std::set<unsigned int> pset;
	std::vector<Term>::iterator ri = rt.begin(), re = rt.end();
	for (; ri != re; ++ri)
	{
		pset.insert( ri->pos());
	}
	std::map<unsigned int, unsigned int> posmap;
	unsigned int pcnt = 0;
	std::set<unsigned int>::const_reverse_iterator pi = pset.rbegin(), pe = pset.rend();
	for (; pi != pe; ++pi)
	{
		posmap[ *pi] = ++pcnt;
	}
	for (ri = rt.begin(); ri != re; ++ri)
	{
		ri->setPos( posmap[ ri->pos()]);
	}
}


