/*
---------------------------------------------------------------------
    The C++ library strus implements basic operations to build
    a search engine for structured search on unstructured data.

    Copyright (C) 2015 Patrick Frey

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License as published by the Free Software Foundation; either
    version 3 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

--------------------------------------------------------------------

	The latest version of strus can be found at 'http://github.com/patrickfrey/strus'
	For documentation see 'http://patrickfrey.github.com/strus'

--------------------------------------------------------------------
*/
/// \brief Exported functions of the strus analyzer vector space model aggregator functions library
/// \file aggregator_vsm.hpp
#ifndef _STRUS_ANALYZER_AGGREGATOR_VSM_LIB_HPP_INCLUDED
#define _STRUS_ANALYZER_AGGREGATOR_VSM_LIB_HPP_INCLUDED

/// \brief strus toplevel namespace
namespace strus
{

/// \brief Forward declaration
class AggregatorFunctionInterface;
/// \brief Forward declaration
class AnalyzerErrorBufferInterface;

/// \brief Get the aggregator function type for the cosine measure normalization factor
/// \return the aggregator function
AggregatorFunctionInterface* createAggregator_sumSquareTf( AnalyzerErrorBufferInterface* errorhnd);

}//namespace
#endif

