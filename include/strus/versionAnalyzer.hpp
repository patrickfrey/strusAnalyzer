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
/// \brief Version of the strus analyzer project
/// \file versionAnalyzer.hpp
#ifndef _STRUS_ANALYZER_VERSION_HPP_INCLUDED
#define _STRUS_ANALYZER_VERSION_HPP_INCLUDED

/// \brief strus toplevel namespace
namespace strus
{

/// \brief Version number of the strus analyzer
#define STRUS_ANALYZER_VERSION (\
	0 * 1000000\
	+ 1 * 10000\
	+ 2\
)

/// \brief Major version number of the strus analyzer
#define STRUS_ANALYZER_VERSION_MAJOR 0
/// \brief Minor version number of the strus analyzer
#define STRUS_ANALYZER_VERSION_MINOR 1

/// \brief The version of the strus core (storage) as string
#define STRUS_ANALYZER_VERSION_STRING "0.1.2"

}//namespace
#endif
