/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Interface for an aggregator function instance
/// \file aggregatorFunctionInstanceInterface.hpp
#ifndef _STRUS_ANALYZER_AGGREGATOR_FUNCTION_INSTANCE_INTERFACE_HPP_INCLUDED
#define _STRUS_ANALYZER_AGGREGATOR_FUNCTION_INSTANCE_INTERFACE_HPP_INCLUDED
#include "strus/analyzer/document.hpp"
#include "strus/analyzer/functionView.hpp"
#include "strus/numericVariant.hpp"

/// \brief strus toplevel namespace
namespace strus
{

/// \class AggregatorFunctionInstanceInterface
/// \brief Interface for a parameterized aggregator function
class AggregatorFunctionInstanceInterface
{
public:
	/// \brief Destructor
	virtual ~AggregatorFunctionInstanceInterface(){}

	/// \brief Aggregator function for document statistics
	/// \param[in] document document to inspect
	/// \return aggregated value
	virtual NumericVariant evaluate( const analyzer::Document& document) const=0;

	/// \brief Get the definition of the function as structure for introspection
	/// \return structure for introspection
	virtual analyzer::FunctionView view() const=0;
};

}//namespace
#endif

