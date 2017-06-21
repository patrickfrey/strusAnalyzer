/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Expression of a query terms as result of a query analysis
/// \file query.hpp
#ifndef _STRUS_ANALYZER_QUERY_TERM_EXPRESSION_HPP_INCLUDED
#define _STRUS_ANALYZER_QUERY_TERM_EXPRESSION_HPP_INCLUDED
#include "strus/analyzer/queryTerm.hpp"
#include <string>
#include <vector>

/// \brief strus toplevel namespace
namespace strus {
/// \brief analyzer parameter and return value objects namespace
namespace analyzer {

/// \brief Expression of a query terms as result of a query analysis
class QueryTermExpression
{
public:
	/// \brief Default constructor
	QueryTermExpression(){}
	/// \brief Copy constructor
	QueryTermExpression( const QueryTermExpression& o)
		:m_terms(o.m_terms)
		,m_instructions(o.m_instructions){}

	/// \brief Query instruction
	class Instruction
	{
	public:
		enum OpCode {Term,Operator};
		static const char* opCodeName( OpCode i)
		{
			static const char* ar[] = {"term","op"};
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
		friend class QueryTermExpression;
		OpCode m_opCode;
		unsigned int m_idx;
		unsigned int m_nofOperands;
	};

	/// \brief Get the list of query instructions
	/// \return the list
	const std::vector<Instruction>& instructions() const	{return m_instructions;}

	/// \brief Get the argument of a term instruction
	/// \param[in] idx index of instruction refering to a term (Instruction::idx)
	const analyzer::QueryTerm& term( unsigned int idx) const
	{
		return m_terms[ idx];
	}

	/// \brief Add a search index term to the query
	/// \param[in] term term to add
	void pushTerm( const analyzer::QueryTerm& term)
	{
		m_instructions.push_back( Instruction( Instruction::Term, m_terms.size()));
		m_terms.push_back( term);
	}

	/// \brief Add an instruction
	/// \param[in] operatorId id of the operator
	/// \param[in] nofOperands number of operands
	void pushOperator( unsigned int operatorId, unsigned int nofOperands)
	{
		m_instructions.push_back( Instruction( Instruction::Operator, operatorId, nofOperands));
	}

private:
	std::vector<analyzer::QueryTerm> m_terms;
	std::vector<Instruction> m_instructions;
};

}}//namespace
#endif

