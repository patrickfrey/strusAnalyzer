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
#include "strus/analyzer/term.hpp"
#include "strus/analyzer/attribute.hpp"
#include "strus/analyzer/metaData.hpp"
#include <string>
#include <vector>

/// \brief strus toplevel namespace
namespace strus {
/// \brief analyzer parameter and return value objects namespace
namespace analyzer {

/// \brief Structure of a query created as result of a query analysis
class Query
{
public:
	/// \brief Default constructor
	Query(){}
	/// \brief Copy constructor
	Query( const Query& o)
		:m_metadata(o.m_metadata)
		,m_searchIndexTerms(o.m_searchIndexTerms)
		,m_elements(o.m_elements)
		,m_operatorNames(o.m_operatorNames)
		,m_instructions(o.m_instructions){}

	/// \brief Get a metadata element by index
	const MetaData& metadata( std::size_t idx) const	{return m_metadata[ idx];}
	/// \brief Get a search index term by index
	const Term& searchIndexTerm( std::size_t idx) const	{return m_searchIndexTerms[ idx];}
	/// \brief Get a operator name by index
	const std::string& operatorName( std::size_t idx) const	{return m_operatorNames[ idx];}

	/// \brief Query instruction
	class Instruction
	{
	public:
		enum OpCode {PushMetaData,PushSearchIndexTerm,Operator};

		Instruction( OpCode opCode_, std::size_t idx_, std::size_t nofOperands_=0)
			:m_opCode(opCode_),m_idx(idx_),m_nofOperands(nofOperands_){}
		Instruction( const Instruction& o)
			:m_opCode(o.m_opCode),m_idx(o.m_idx),m_nofOperands(o.m_nofOperands){}

		/// \brief Opcode identifier
		OpCode opCode() const			{return m_opCode;}
		/// \brief Index of the element in the associated list to retrieve with searchIndexTerm(std::size_t),metadata(std::size_t) or operatorName(std::size_t)
		unsigned int idx() const		{return m_idx;}
		/// \brief Number of operands
		unsigned int nofOperands() const	{return m_nofOperands;}

	private:
		OpCode m_opCode;
		unsigned int m_idx;
		unsigned int m_nofOperands;
	};

	/// \brief Query element
	class Element
	{
	public:
		enum Type {MetaData,SearchIndexTerm};

		Element( Type type_, unsigned int idx_, unsigned int position_, unsigned int fieldno_)
			:m_type(type_),m_idx(idx_),m_position(position_),m_fieldno(fieldno_){}
		Element( const Element& o)
			:m_type(o.m_type),m_idx(o.m_idx),m_position(o.m_position),m_fieldno(o.m_fieldno){}

		/// \brief Type identifier referencing the list this element belongs to
		Type type() const			{return m_type;}
		/// \brief Index of the element in the associated list to retrieve with searchIndexTerm(std::size_t) or metadata(std::size_t)
		unsigned int idx() const		{return m_idx;}
		/// \brief Query element ordinal position
		unsigned int position() const		{return m_position;}
		/// \brief Query field number
		unsigned int fieldno() const		{return m_fieldno;}

	private:
		Type m_type;
		unsigned int m_idx;
		unsigned int m_position;
		unsigned int m_fieldno;
	};

	/// \brief Get the list of query meta data elements
	/// \return the list
	const std::vector<Element>& elements() const			{return m_elements;}

	/// \brief Get the list of query instructions
	/// \return the list
	const std::vector<Instruction>& instructions() const		{return m_instructions;}

	/* Add query elements (tokenized and normalized items) */
	/// \brief Add a search index term to the query
	void addSearchIndexTerm( unsigned int fieldno, const analyzer::Term& term)
	{
		m_elements.push_back( Element( Element::SearchIndexTerm, m_searchIndexTerms.size(), term.pos(), fieldno));
		m_searchIndexTerms.push_back( term);
	}

	/// \brief Add a meta data element to the query
	void addMetaData( unsigned int fieldno, unsigned int position, const analyzer::MetaData& elem)
	{
		m_elements.push_back( Element( Element::MetaData, m_metadata.size(), position, fieldno));
		m_metadata.push_back( elem);
	}

	/* Build query structure */
	/// \brief Add an instruction
	void pushOperator( const std::string& name, unsigned int nofOperands)
	{
		m_instructions.push_back( Instruction( Instruction::Operator, m_operatorNames.size(), nofOperands));
		m_operatorNames.push_back( name);
	}

	/// \brief Add a meta data operand
	void pushMetaDataOperand( unsigned int idx)
	{
		m_instructions.push_back( Instruction( Instruction::PushMetaData, idx));
	}

	/// \brief Add a meta data operand
	void pushSearchIndexTermOperand( unsigned int idx)
	{
		m_instructions.push_back( Instruction( Instruction::PushSearchIndexTerm, idx));
	}

private:
	std::vector<analyzer::MetaData> m_metadata;
	std::vector<analyzer::Term> m_searchIndexTerms;
	std::vector<Element> m_elements;
	std::vector<std::string> m_operatorNames;
	std::vector<Instruction> m_instructions;
};

}}//namespace
#endif

