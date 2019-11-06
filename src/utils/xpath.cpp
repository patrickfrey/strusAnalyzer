/*
 * Copyright (c) 2019 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "private/xpath.hpp"

using namespace strus;

std::string strus::joinXPathExpression( const std::string& parent, const std::string& follow)
{
	if (follow.empty())
	{
		return parent;
	}
	else if (parent.empty())
	{
		return follow;
	}
	else if (follow[0] == '/')
	{
		//... follow with starting '/'
		std::size_t pi = parent.size();
		for (; pi > 0 && parent[ pi-1] == '/'; --pi){}
		if (pi + 1 < parent.size())
		{
			//... parent ending with '//'
			std::size_t fi = 0;
			for (; fi < follow.size() && follow[fi] == '/'; ++fi){}
			return parent + std::string( follow.c_str()+fi, follow.size()-fi);
		}
		else
		{
			//... parent cut single '/' and append follow with starting '/'
			return std::string( parent.c_str(), pi) + follow;
		}
	}
	else if (parent[ parent.size()-1] == '/')
	{
		//... parent with ending '/' but follow not staring with '/'
		return parent + follow;
	}
	else
	{
		//... parent without ending '/' and follow without starting '/'
		return parent + "/" + follow;
	}
}

std::string strus::xpathStartStructurePath( const std::string& selectexpr)
{
	std::size_t si = selectexpr.size();
	for (; si > 0 && ((unsigned char)selectexpr[si-1] <= 32 || selectexpr[si-1] == '/'); --si){}
	if (si > 0 && selectexpr[si-1] == ')')
	{
		for (--si; si > 0 && (unsigned char)selectexpr[si-1] <= 32; --si){}
		if (si > 0 && selectexpr[si-1] == '(')
		{
			--si;
			for (--si; si > 0 && (unsigned char)selectexpr[si-1] <= 32; --si){}
			return std::string( selectexpr.c_str(), si);
		}
	}
	return selectexpr;
}

std::string strus::xpathEndStructurePath( const std::string& selectexpr)
{
	std::string rt = xpathStartStructurePath( selectexpr);
	if (rt.empty() || rt[ rt.size()-1] != '~')
	{
		rt.push_back( '~');
	}
	return rt;
}


