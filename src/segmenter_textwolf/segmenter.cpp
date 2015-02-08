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
}

SegmenterInstanceInterface* Segmenter::createInstance( const char* source)
{
	return new SegmenterInstance( &m_automaton, source);
}


