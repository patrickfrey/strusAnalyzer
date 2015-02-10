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
#include "segmenterInstance.hpp"
#include "textwolf/xmlpathautomatonparse.hpp"
#include "textwolf/xmlpathselect.hpp"
#include "textwolf/charset.hpp"

using namespace strus;

typedef textwolf::XMLScanner<
		char const*,
		textwolf::charset::UTF8,
		textwolf::charset::UTF8,
		std::string
	> XMLScanner;

typedef textwolf::XMLPathSelect<
		textwolf::charset::UTF8
	> XMLPathSelect;

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

static void updateSelectorTypeMap(
		const textwolf::XMLPathSelectAutomatonParser<>& atm,
		std::map<int,SegmenterInterface::SelectorType>& selectorTypeMap,
		int id,
		const std::string& expression)
{
	XMLPathSelect xs( &atm);
	std::vector<std::string> tags;

	enum ExpressionClass
	{
		ContentSelection,
		TagSelection,
		AttributeSelection
	};
	ExpressionClass expressionClass = TagSelection;
		
	char const* si = expression.c_str();
	char const* se = si + expression.size();

	// Determine the ExpressionClass
	// and get the tag selection state of the last tag node of the expression:
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
				expressionClass = AttributeSelection;
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
					expressionClass = ContentSelection;
				}
			}
			continue;
		}
		throw std::runtime_error( std::string( "error in path expression at '") + si + "' (expression '" + expression + "')");
	}
	switch (expressionClass)
	{
		case ContentSelection:
		{
			selectorTypeMap[ id] = SegmenterInterface::Content;
			break;
		}
		case TagSelection:
		{
			// Tag selection generates a content element:
			selectorTypeMap[ id] = SegmenterInterface::Content;

			// Get the xpath selection to the state of the tag selection of the expression:
			std::vector<std::string>::const_iterator ti = tags.begin(), te = tags.end();
			for (; ti != te; ++ti)
			{
				XMLPathSelect::iterator
					itr = xs.push( XMLScanner::OpenTag, ti->c_str(), ti->size()),end=xs.end();
				for (; itr != end; ++itr){}
			}

			// Evaluate if there are attribute selections defined for this tag selection.
			// If yes mark them as being bound to this tag selection (Predecessor):
			std::vector<unsigned int> fbuf;
			xs.getTokenTypeMatchingStates( XMLScanner::TagAttribName, false, fbuf);
			std::vector<unsigned int>::const_iterator fi = fbuf.begin(), fe = fbuf.end();
			for (; fi != fe; ++fi)
			{
				std::vector<int> ebuf;
				atm.getEmmitedTokens( *fi, XMLScanner::TagAttribValue, ebuf);
				std::vector<int>::const_iterator ei = ebuf.begin(), ee = ebuf.end();
				for (; ei != ee; ++ei)
				{
					selectorTypeMap[ *ei] = SegmenterInterface::AnnotationPredecessor;
				}
			}
			break;
		}
		case AttributeSelection:
		{
			if (selectorTypeMap[ id] != SegmenterInterface::AnnotationPredecessor)
			{
				selectorTypeMap[ id] = SegmenterInterface::AnnotationSuccessor;

				bool tagOutput = false;

				// Get the xpath selection to the state of the tag selection of the expression
				// and determine if the tag is selected:
				std::vector<std::string>::const_iterator ti = tags.begin(), te = tags.end();
				for (; ti != te; ++ti)
				{
					XMLPathSelect::iterator
						itr = xs.push( XMLScanner::OpenTag, ti->c_str(), ti->size()),end=xs.end();
					tagOutput = (itr != end);
					for (; itr != end; ++itr){}
				}
				// If the tag the selected attribute belongs to is selected we create
				// An annotation type bound to the tag of the attribute. Otherwise
				// to the first content element (immediate successor):
				if (tagOutput)
				{
					selectorTypeMap[ id] = SegmenterInterface::AnnotationPredecessor;
				}
				else
				{
					selectorTypeMap[ id] = SegmenterInterface::AnnotationSuccessor;
				}
			}
			break;
		}
	}
}

SegmenterInterface::SelectorType Segmenter::getSelectorType( int id) const
{
	std::map<int,SelectorType>::const_iterator si = m_selectorTypeMap.find( id);
	if (si == m_selectorTypeMap.end())
	{
		throw std::runtime_error( "expression with this id not defined in textwolf XML segmenter");
	}
	return si->second;
}

void Segmenter::defineSelectorExpression( int id, const std::string& expression)
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
	updateSelectorTypeMap( m_automaton, m_selectorTypeMap, id, expression);
}

SegmenterInstanceInterface* Segmenter::createInstance( const std::string& source) const
{
	return new SegmenterInstance( &m_automaton, source.c_str());
}


