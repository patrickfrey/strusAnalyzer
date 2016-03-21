/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
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
	virtual AggregatorFunctionInstanceInterface* createInstance( const std::vector<std::string>& args) const=0;

	/// \brief Get a description of the function for user help
	/// \return the description
	virtual const char* getDescription() const=0;
};

}//namespace
#endif

