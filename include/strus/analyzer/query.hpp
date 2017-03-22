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
		,m_instructions(o.m_instructions){}

	/// \brief Get all metadata elements
	const std::vector<MetaData>& metadata() const		{return m_metadata;}
	/// \brief Get all search index terms
	const std::vector<Term>& searchIndexTerms() const	{return m_searchIndexTerms;}
	/// \brief Get a metadata element by index
	const MetaData& metadata( std::size_t idx) const	{return m_metadata[ idx];}
	/// \brief Get a search index term by index
	const Term& searchIndexTerm( std::size_t idx) const	{return m_searchIndexTerms[ idx];}
	/// \brief Test if query is empty
	bool empty() const					{return m_elements.empty();}

	/// \brief Query instruction
	class Instruction
	{
	public:
		enum OpCode {PushMetaData,PushSearchIndexTerm,Operator};
		static const char* opCodeName( OpCode i)
		{
			static const char* ar[] = {"meta","term","op"};
			return ar[i];
		}

		Instruction( OpCode opCode_, std::size_t idx_, std::size_t nofOperands_=0)
			:m_opCode(opCode_),m_idx(idx_),m_nofOperands(nofOperands_){}
		Instruction( const Instruction& o)
			:m_opCode(o.m_opCode),m_idx(o.m_idx),m_nofOperands(o.m_nofOperands){}

		/// \brief Opcode identifier
		OpCode opCode() const				{return m_opCode;}
		/// \brief Index of the element in the associated list to retrieve with searchIndexTerm(std::size_t),metadata(std::size_t) of the operatorId
		unsigned int idx() const			{return m_idx;}
		/// \brief Number of operands
		unsigned int nofOperands() const		{return m_nofOperands;}

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
		static const char* typeName( Type i)
		{
			static const char* ar[] = {"meta","term"};
			return ar[i];
		}

		Element( Type type_, unsigned int idx_, unsigned int position_, unsigned int length_, unsigned int fieldNo_)
			:m_type(type_),m_idx(idx_),m_position(position_),m_length(length_),m_fieldNo(fieldNo_){}
		Element( const Element& o)
			:m_type(o.m_type),m_idx(o.m_idx),m_position(o.m_position),m_length(o.m_length),m_fieldNo(o.m_fieldNo){}

		/// \brief Type identifier referencing the list this element belongs to
		Type type() const				{return m_type;}
		/// \brief Index of the element in the associated list to retrieve with searchIndexTerm(std::size_t) or metadata(std::size_t)
		unsigned int idx() const			{return m_idx;}
		/// \brief Query element ordinal position
		unsigned int position() const			{return m_position;}
		/// \brief Query element ordinal position length
		unsigned int length() const			{return m_length;}
		/// \brief Query field number
		unsigned int fieldNo() const			{return m_fieldNo;}

	private:
		Type m_type;
		unsigned int m_idx;
		unsigned int m_position;
		unsigned int m_length;
		unsigned int m_fieldNo;
	};

	/// \brief Get the list of query elements
	/// \return the list
	const std::vector<Element>& elements() const		{return m_elements;}

	/// \brief Get the list of query instructions
	/// \return the list
	const std::vector<Instruction>& instructions() const	{return m_instructions;}

	/* Add query elements (tokenized and normalized items) */
	/// \brief Add a search index term to the query
	void addSearchIndexTerm( unsigned int fieldNo, const analyzer::Term& term)
	{
		m_elements.push_back( Element( Element::SearchIndexTerm, m_searchIndexTerms.size(), term.pos(), term.len(), fieldNo));
		m_searchIndexTerms.push_back( term);
	}

	/// \brief Add a meta data element to the query
	void addMetaData( unsigned int fieldNo, unsigned int position, const analyzer::MetaData& elem)
	{
		m_elements.push_back( Element( Element::MetaData, m_metadata.size(), position, 1, fieldNo));
		m_metadata.push_back( elem);
	}

	/* Build query structure (list of instructions) */
	/// \brief Add an instruction
	void pushOperator( unsigned int operatorId, unsigned int nofOperands)
	{
		m_instructions.push_back( Instruction( Instruction::Operator, operatorId, nofOperands));
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
	std::vector<Instruction> m_instructions;
};

}}//namespace
#endif

