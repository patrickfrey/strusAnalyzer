/*
 * Copyright (c) 2018 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Structures describing the internal representation of an element in a query analyzer
/// \note The internal representation may not be suitable for reconstructing the object
/// \file queryElementView.hpp
#ifndef _STRUS_ANALYZER_QUERY_ELEMENT_VIEW_HPP_INCLUDED
#define _STRUS_ANALYZER_QUERY_ELEMENT_VIEW_HPP_INCLUDED
#include "strus/analyzer/functionView.hpp"

/// \brief strus toplevel namespace
namespace strus {
/// \brief analyzer parameter and return value objects namespace
namespace analyzer {

/// \brief Structure describing the internal representation of a feature in the document analyzer
/// \note The internal representation may not be suitable for reconstructing the object
class QueryElementView
{
public:
	/// \brief Default constructor
	QueryElementView(){}
	/// \brief Copy constructor
	QueryElementView( const QueryElementView& o)
		:m_type(o.m_type),m_field(o.m_field),m_tokenizer(o.m_tokenizer),m_normalizer(o.m_normalizer){}

	/// \brief Constructor
	/// \param[in] type_ name of the function
	/// \param[in] field_ list of named parameters
	/// \param[in] tokenizer_ view of tokenizer
	/// \param[in] normalizer_ list of views of normalizers
	QueryElementView( const std::string& type_, const std::string& field_, const FunctionView& tokenizer_, const std::vector<FunctionView>& normalizer_)
		:m_type(type_),m_field(field_),m_tokenizer(tokenizer_),m_normalizer(normalizer_){}

	/// \brief Get the type
	const std::string& type() const				{return m_type;}
	/// \brief Get the name of the query field
	const std::string& field() const			{return m_field;}
	/// \brief Get the tokenizer
	const FunctionView& tokenizer() const			{return m_tokenizer;}
	/// \brief Get the list of normalizers
	const std::vector<FunctionView>& normalizer() const	{return m_normalizer;}

private:
	std::string m_type;
	std::string m_field;
	FunctionView m_tokenizer;
	std::vector<FunctionView> m_normalizer;
};

}}//namespace
#endif

