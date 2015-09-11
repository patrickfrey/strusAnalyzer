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
/// \brief Interface for an aggregator function type
/// \file aggregatorFunctionInterface.hpp
#ifndef _STRUS_ANALYZER_AGGREGATOR_INTERFACE_HPP_INCLUDED
#define _STRUS_ANALYZER_AGGREGATOR_INTERFACE_HPP_INCLUDED
#include <vector>
#include <string>

/// \brief strus toplevel namespace
namespace strus
{

/// \brief Forward declaration
class AggregatorFunctionInstanceInterface;
/// \brief Forward declaration
class AnalyzerErrorBufferInterface;

/// \class AggregatorFunctionInterface
/// \brief Interface for the aggregator function constructor
class AggregatorFunctionInterface
{
public:
	/// \brief Destructor
	virtual ~AggregatorFunctionInterface(){}

	/// \brief Create a parameterized aggregator function instance
	/// \param[in] args arguments for the aggregator function
	/// \param[in] errorhnd analyzer error buffer interface for reporting exeptions and errors
	virtual AggregatorFunctionInstanceInterface* createInstance( const std::vector<std::string>& args, AnalyzerErrorBufferInterface* errorhnd) const=0;
};

}//namespace
#endif

