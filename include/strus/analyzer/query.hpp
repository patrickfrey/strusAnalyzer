/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Structure of a query as result of query analysis
/// \file query.hpp
#ifndef _STRUS_ANALYZER_QUERY_HPP_INCLUDED
#define _STRUS_ANALYZER_QUERY_HPP_INCLUDED
#include "strus/numericVariant.hpp"
#include "strus/analyzer/term.hpp"
#include "strus/analyzer/attribute.hpp"
#include "strus/analyzer/metaData.hpp"
#include <string>
#include <vector>

/// \brief strus toplevel namespace
namespace strus {
/// \brief analyzer parameter and return value objects namespace
namespace analyzer {

/// \brief Structure of a document created as result of a document analysis
class Query
{
public:
	/// \brief Default constructor
	Query(){}
	/// \brief Copy constructor
	Query( const Query& o)
		:m_metadata(o.m_metadata)
		,m_searchIndexTerms(o.m_searchIndexTerms)
		,m_forwardIndexTerms(o.m_forwardIndexTerms)
		,m_groupNames(o.m_groupNames){}

	/// \brief Get a metadata element by index
	const MetaData& metadata( std::size_t idx) const	{return m_metadata[ idx];}
	/// \brief Get a search index term by index
	const Term& searchIndexTerm( std::size_t idx) const	{return m_searchIndexTerms[ idx];}
	/// \brief Get a forward index term by index
	const Term& forwardIndexTerm( std::size_t idx) const	{return m_forwardIndexTerms[ idx];}
	/// \brief Get a group name by index
	const std::string& groupName( std::size_t idx) const	{return m_groupNames[ idx];}

	class Element
	{
	public:
		enum Type {MetaData,SearchIndex,ForwardIndex,Group};
		Element( Type type_, std::size_t idx_, std::size_t size_=0)
			:m_type(type_),m_idx(idx_),m_size(size_){}
		Element( const Element& o)
			:m_type(o.m_type),m_idx(o.m_idx),m_size(o.m_size){}

		/// \brief Type identifier referencing the list this element belongs to
		Type type() const		{return m_type;}
		/// \brief Index of the element in the associated list to retrieve with searchIndexTerm(std::size_t),forwardIndexTerm(std::size_t),metadata(std::size_t) or groupName(std::size_t)
		std::size_t idx() const		{return m_idx;}
		/// \brief Number of child elements if this is a 'Element::Group', 0 otherwise
		std::size_t size() const	{return m_size;}

	private:
		Type m_type;
		std::size_t m_idx;
		std::size_t m_size;
	};

	/// \brief Get the list of query elements
	/// \return the list
	std::vector<Element> elements() const	{return m_elements;}

private:
	std::vector<MetaData> m_metadata;
	std::vector<Term> m_searchIndexTerms;
	std::vector<Term> m_forwardIndexTerms;
	std::vector<std::string> m_groupNames;
	std::vector<Element> m_elements;
};

}}//namespace
#endif

