/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "private/xpathAutomaton.hpp"
#include "private/errorUtils.hpp"
#include "private/internationalization.hpp"
#include <cstdlib>
#include <setjmp.h>

using namespace strus;

XPathAutomatonContext::XPathAutomatonContext( const Automaton* automaton_)
	:m_automaton(automaton_)
	,m_pathselect(automaton_)
{
	m_selitr = m_selend = m_pathselect.end();
}

void XPathAutomatonContext::putElement(
	const textwolf::XMLScannerBase::ElementType& elemtype,
	const char* elem_,
	std::size_t elemsize_)
{
	m_selitr = m_pathselect.push( elemtype, elem_, elemsize_);
	m_selend = m_pathselect.end();
}

bool XPathAutomatonContext::getNext( int& id)
{
	if (m_selitr == m_selend) return false;
	id = *m_selitr;
	++m_selitr;
	return true;
}


static bool isTagNameChar( char ch)
{
	if (ch == ':' || ch == '@' || ch == '/' || ch == '[' || ch == ']' || ch == '(' || ch == ')') return false;
	return true;
}

static bool isSpace( char ch)
{
	return (unsigned char)ch <= 32;
}

static void skipSpaces( char const*& si, const char* se)
{
	for (; si != se && isSpace(*si); ++si){}
}


enum ExpressionClass
{
	ContentSelection,
	TagSelection,
	AttributeSelection
};

// Determine the ExpressionClass
// and get the tag selection state of the last tag node of the expression:
static ExpressionClass getExpressionClass( const std::string& expression, std::vector<std::string>& tags)
{
	ExpressionClass rt = TagSelection;
	char const* si = expression.c_str();
	char const* se = si + expression.size();

	while (si != se)
	{
		for (; si != se && (*si == '/' || isSpace(*si)); ++si){}
		if (si == se) break;
		if (si != se && *si == '[')
		{
			// Skip attribute conditions:
			++si;
			for (;si != se && *si != ']'; ++si){}
			if  (si != se) ++si;
			continue;
		}
		if (si != se && isTagNameChar(*si))
		{
			// Handle open tag into:
			char const* ti = si;
			for (++si; si != se && isTagNameChar(*si); ++si){}
			tags.push_back( std::string( ti, si-ti));
			continue;
		}
		if (si != se && *si == '@')
		{
			// Handle attribute selection at the end of the expression:
			++si;
			skipSpaces( si, se);
			for (; si != se && isTagNameChar(*si); ++si){}
			if (si == se)
			{
				rt = AttributeSelection;
			}
			continue;
		}
		if (si != se && *si == '(')
		{
			++si;
			skipSpaces( si, se);
			if (si != se && *si == ')')
			{
				++si;
				skipSpaces( si, se);
				if (si == se)
				{
					rt = ContentSelection;
				}
			}
			continue;
		}
		throw strus::runtime_error( _TXT("error in path expression at '%s' (expression '%s')"), si, expression.c_str());
	}
	return rt;
}

void XPathAutomaton::addExpression( int id, const std::string& expression)
{
	int errorpos = m_automaton.addExpression( id, expression.c_str(), expression.size());
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
		throw strus::runtime_error( _TXT("error in selection expression '%s' at %s"), expression.c_str(), locstr.c_str());
	}
}

void XPathAutomaton::defineSelectorExpression( int id, const std::string& expression)
{
	addExpression( id, expression);
}


void XPathAutomaton::defineSubSection( int startId, int endId, const std::string& expression)
{
	std::vector<std::string> tags;
	if (getExpressionClass( expression, tags) != TagSelection)
	{
		throw strus::runtime_error( _TXT("tag selection expected for defining a sub section of the document: '%s'"), expression.c_str());
	}
	addExpression( startId, expression);
	addExpression( endId, expression + "~");
}

XPathAutomatonContext XPathAutomaton::createContext() const
{
	return XPathAutomatonContext( &m_automaton);
}





