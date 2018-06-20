/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef _STRUS_ANALYZER_BIND_TERM_HPP_INCLUDED
#define _STRUS_ANALYZER_BIND_TERM_HPP_INCLUDED
#include "strus/analyzer/positionBind.hpp"
#include <string>
#include <vector>

namespace strus
{

class BindTerm
{
public:
	BindTerm( int seg_, int ofs_, int endofs_, int ordlen_, int priority_, analyzer::PositionBind posbind_, const std::string& type_, const std::string& value_)
		:m_seg(seg_),m_ofs(ofs_),m_endofs(endofs_),m_ordlen(ordlen_),m_priority(priority_),m_posbind(posbind_),m_type(type_),m_value(value_){}
#if __cplusplus >= 201103L
	BindTerm( BindTerm&& ) = default;
	BindTerm( const BindTerm& ) = default;
	BindTerm& operator= ( BindTerm&& ) = default;
	BindTerm& operator= ( const BindTerm& ) = default;
#else
	BindTerm( const BindTerm& o)
		:m_seg(o.m_seg),m_ofs(o.m_ofs),m_endofs(o.m_endofs),m_ordlen(o.m_ordlen),m_priority(o.m_priority),m_posbind(o.m_posbind),m_type(o.m_type),m_value(o.m_value){}
#endif
	int seg() const						{return m_seg;}
	int ofs() const						{return m_ofs;}
	int endofs() const					{return m_endofs;}
	int ordlen() const					{return m_ordlen;}
	const std::string& type() const				{return m_type;}
	const std::string& value() const			{return m_value;}
	int priority() const					{return m_priority;}
	analyzer::PositionBind posbind() const			{return m_posbind;}

	bool operator < (const BindTerm& o) const
	{
		return (m_seg == o.m_seg)
			? (
				(m_ofs == o.m_ofs)
				? (
					(m_ordlen == o.m_ordlen)
					? (
						(m_endofs == o.m_endofs)
						? (
							(m_type == o.m_type)
							? (m_value < o.m_value)
							: (m_type < o.m_type)
						)
						: (m_endofs < o.m_endofs)
					)
					: (m_ordlen < o.m_ordlen)
				)
				: (m_ofs < o.m_ofs)
			)
			: (m_seg < o.m_seg);
	}

	static void eliminateCoveredElements( std::vector<BindTerm>& terms);

private:
	int m_seg;
	int m_ofs;
	int m_endofs;
	int m_ordlen;
	int m_priority;
	analyzer::PositionBind m_posbind;
	std::string m_type;
	std::string m_value;
};

class BindLexem
	:public BindTerm
{
public:
	BindLexem( const BindLexem& o)
		:BindTerm(o),m_id(o.m_id){}
	BindLexem( int id_, const BindTerm& term)
		:BindTerm(term),m_id(id_){}

	int id() const		{return m_id;}

	bool operator < (const BindLexem& o) const
	{
		return BindTerm::operator <(o);
	}

private:
	int m_id;
};

}//namespace
#endif

