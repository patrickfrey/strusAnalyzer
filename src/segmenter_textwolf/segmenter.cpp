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
#include "segmenter.hpp"
#include "segmenterContext.hpp"
#include "strus/contentDescriptionInterface.hpp"
#include "textwolf/xmlpathautomatonparse.hpp"
#include "textwolf/charset.hpp"
#include "private/utils.hpp"

using namespace strus;

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
		throw std::runtime_error( std::string( "error in path expression at '") + si + "' (expression '" + expression + "')");
	}
	return rt;
}

void Segmenter::addExpression( int id, const std::string& expression)
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
		throw std::runtime_error( std::string( "error in selection expression '") + expression + "' at " + locstr);
	}
}

void Segmenter::defineSelectorExpression( int id, const std::string& expression)
{
	addExpression( id, expression);
}


void Segmenter::defineSubSection( int startId, int endId, const std::string& expression)
{
	std::vector<std::string> tags;
	if (getExpressionClass( expression, tags) != TagSelection)
	{
		throw std::runtime_error( std::string( "tag selection expected for defining a sub section of the document: '") + expression + "'");
	}
	addExpression( startId, expression);
	addExpression( endId, expression + "~");
}


SegmenterContextInterface* Segmenter::createContext( const ContentDescriptionInterface& descr) const
{
	const char* encoding = descr.getProperty( ContentDescriptionInterface::Encoding);
	if (encoding != 0)
	{
		if (!utils::caseInsensitiveEquals( encoding, "UTF-8"))
		{
			throw std::runtime_error( "the XML segmenter based on textwolf currently supports only UTF-8 as character set encoding");
		}
	}
	return new SegmenterContext( &m_automaton);
}


