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
#include "strus/documentClass.hpp"
#include "strus/analyzerErrorBufferInterface.hpp"
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

void SegmenterInstance::addExpression( int id, const std::string& expression)
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

void SegmenterInstance::defineSelectorExpression( int id, const std::string& expression)
{
	addExpression( id, expression);
}


void SegmenterInstance::defineSubSection( int startId, int endId, const std::string& expression)
{
	std::vector<std::string> tags;
	if (getExpressionClass( expression, tags) != TagSelection)
	{
		throw std::runtime_error( std::string( "tag selection expected for defining a sub section of the document: '") + expression + "'");
	}
	addExpression( startId, expression);
	addExpression( endId, expression + "~");
}


SegmenterContextInterface* SegmenterInstance::createContext( const DocumentClass& dclass) const
{
	try
	{
		typedef textwolf::charset::UTF8 UTF8;
		typedef textwolf::charset::UTF16<textwolf::charset::ByteOrder::BE> UTF16BE;
		typedef textwolf::charset::UTF16<textwolf::charset::ByteOrder::LE> UTF16LE;
		typedef textwolf::charset::UCS2<textwolf::charset::ByteOrder::BE> UCS2BE;
		typedef textwolf::charset::UCS2<textwolf::charset::ByteOrder::LE> UCS2LE;
		typedef textwolf::charset::UCS4<textwolf::charset::ByteOrder::BE> UCS4BE;
		typedef textwolf::charset::UCS4<textwolf::charset::ByteOrder::LE> UCS4LE;
		typedef textwolf::charset::IsoLatin IsoLatin;
		unsigned char codepage = 1;
	
		if (dclass.encoding().empty())
		{
			return new SegmenterContext<UTF8>( m_errorhnd, &m_automaton);
		}
		else
		{
			if (utils::caseInsensitiveStartsWith( dclass.encoding(), "IsoLatin")
			||  utils::caseInsensitiveStartsWith( dclass.encoding(), "ISO-8859"))
			{
				char const* cc = dclass.encoding().c_str() + 8;
				if (*cc == '-')
				{
					++cc;
					if (*cc >= '1' && *cc <= '9' && cc[1] == '\0')
					{
						codepage = *cc - '0';
					}
					else
					{
						m_errorhnd->report( std::string("parse error in character set encoding: '") + dclass.encoding() + "'");
						return 0;
					}
				}
				return new SegmenterContext<IsoLatin>( m_errorhnd, &m_automaton, IsoLatin(codepage));
			}
			else if (utils::caseInsensitiveEquals( dclass.encoding(), "UTF-8"))
			{
				return new SegmenterContext<UTF8>( m_errorhnd, &m_automaton);
			}
			else if (utils::caseInsensitiveEquals( dclass.encoding(), "UTF-16")
			||       utils::caseInsensitiveEquals( dclass.encoding(), "UTF-16BE"))
			{
				return new SegmenterContext<UTF16BE>( m_errorhnd, &m_automaton);
			}
			else if (utils::caseInsensitiveEquals( dclass.encoding(), "UTF-16LE"))
			{
				return new SegmenterContext<UTF16LE>( m_errorhnd, &m_automaton);
			}
			else if (utils::caseInsensitiveEquals( dclass.encoding(), "UCS-2")
			||       utils::caseInsensitiveEquals( dclass.encoding(), "UCS-2BE"))
			{
				return new SegmenterContext<UCS2BE>( m_errorhnd, &m_automaton);
			}
			else if (utils::caseInsensitiveEquals( dclass.encoding(), "UCS-2LE"))
			{
				return new SegmenterContext<UCS2LE>( m_errorhnd, &m_automaton);
			}
			else if (utils::caseInsensitiveEquals( dclass.encoding(), "UCS-4")
			||       utils::caseInsensitiveEquals( dclass.encoding(), "UCS-4BE")
			||       utils::caseInsensitiveEquals( dclass.encoding(), "UTF-32")
			||       utils::caseInsensitiveEquals( dclass.encoding(), "UTF-32BE"))
			{
				return new SegmenterContext<UCS4BE>( m_errorhnd, &m_automaton);
			}
			else if (utils::caseInsensitiveEquals( dclass.encoding(), "UCS-4LE")
			||       utils::caseInsensitiveEquals( dclass.encoding(), "UTF-32LE"))
			{
				return new SegmenterContext<UCS4LE>( m_errorhnd, &m_automaton);
			}
			else
			{
				m_errorhnd->report( "the XML segmenter based on textwolf currently supports only UTF-8,UTF-16BE,UTF-16LE,UTF-32BE,UCS-4BE,UTF-32LE,UCS-4LE and ISO-8859 (code pages 1 to 9) as character set encoding");
				return 0;
			}
		}
	}
	catch (const std::runtime_error& err)
	{
		m_errorhnd->report( std::string( err.what()) + " in 'textwolf' segmenter");
		return 0;
	}
	catch (const std::bad_alloc&)
	{
		m_errorhnd->report( "out of memory in 'textwolf' segmenter");
		return 0;
	}
	catch (const std::exception& err)
	{
		m_errorhnd->report( std::string(err.what()) + " uncaught exception in 'textwolf' segmenter");
		return 0;
	}
}

SegmenterInstanceInterface* Segmenter::createInstance( AnalyzerErrorBufferInterface* errorhnd) const
{
	try
	{
		return new SegmenterInstance( errorhnd);
	}
	catch (const std::runtime_error& err)
	{
		errorhnd->report( std::string( err.what()) + " in 'textwolf' segmenter");
		return 0;
	}
	catch (const std::bad_alloc&)
	{
		errorhnd->report( "out of memory in 'textwolf' segmenter");
		return 0;
	}
	catch (const std::exception& err)
	{
		errorhnd->report( std::string(err.what()) + " uncaught exception in 'textwolf' segmenter");
		return 0;
	}
}


