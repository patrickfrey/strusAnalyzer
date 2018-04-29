/*
 * Copyright (c) 2018 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Structures describing the internal representation of an aggregator in a document analyzer
/// \note The internal representation may not be suitable for reconstructing the object
/// \file aggregatorView.hpp
#ifndef _STRUS_ANALYZER_DOCUMENT_AGGREGATOR_VIEW_HPP_INCLUDED
#define _STRUS_ANALYZER_DOCUMENT_AGGREGATOR_VIEW_HPP_INCLUDED
#include "strus/analyzer/functionView.hpp"

/// \brief strus toplevel namespace
namespace strus {
/// \brief analyzer parameter and return value objects namespace
namespace analyzer {

/// \brief Structures describing the internal representation of an aggregator function in a document analyzer
/// \note The internal representation may not be suitable for reconstructing the object
class AggregatorView
{
public:
	/// \brief Default constructor
	AggregatorView(){}

	/// \brief Copy constructor
	AggregatorView( const AggregatorView& o)
		:m_type(o.m_type),m_function(o.m_function){}

	/// \brief Constructor
	/// \param[in] type_ name of the meta data element created
	/// \param[in] function_ aggregating function
	AggregatorView( const std::string& type_, const FunctionView& function_)
		:m_type(type_),m_function(function_){}

	/// \brief Get the type
	const std::string& type() const				{return m_type;}
	/// \brief Get the function
	const FunctionView& function() const			{return m_function;}

private:
	std::string m_type;
	FunctionView m_function;
};

}}//namespace
#endif

