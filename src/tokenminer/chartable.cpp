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
#include "chartable.hpp"
#include <utility>

using namespace strus;

CharTable::CharTable( const char* op, bool isInverse)
{
	std::size_t ii;
	for (ii=0; ii<=32; ++ii) m_ar[ii] = false;
	for (ii=33; ii<sizeof(m_ar); ++ii) m_ar[ii] = isInverse;
	for (ii=0; op[ii]; ++ii)
	{
		if (op[ii] == '.' && op[ii+1] == '.')
		{
			unsigned char hi = op[ii+2]?(unsigned char)op[ii+2]:255;
			unsigned char lo = (ii>0)?(unsigned char)op[ii-1]:1;
			if (hi < lo)
			{
				unsigned char tmp = hi;
				hi = lo;
				lo = tmp; //... swapped 'hi' and 'lo'
			}
			for (++lo; lo<=hi; ++lo)
			{
				m_ar[ lo] = !isInverse;
			}
			ii += 2;
		}
		m_ar[(unsigned char)(op[ii])] = !isInverse;
	}
}

