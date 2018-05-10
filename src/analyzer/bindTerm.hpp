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

namespace strus
{

class BindTerm
{
public:
	BindTerm( const BindTerm& o)
		:m_seg(o.m_seg),m_ofs(o.m_ofs),m_len(o.m_len),m_posbind(o.m_posbind),m_type(o.m_type),m_value(o.m_value){}
	BindTerm( unsigned int seg_, unsigned int ofs_, unsigned int len_, analyzer::PositionBind posbind_, const std::string& type_, const std::string& value_)
		:m_seg(seg_),m_ofs(ofs_),m_len(len_),m_posbind(posbind_),m_type(type_),m_value(value_){}

	unsigned int seg() const				{return m_seg;}
	unsigned int ofs() const				{return m_ofs;}
	unsigned int len() const				{return m_len;}
	const std::string& type() const				{return m_type;}
	const std::string& value() const			{return m_value;}
	analyzer::PositionBind posbind() const			{return m_posbind;}

	bool operator < (const BindTerm& o) const
	{
		return (m_seg == o.m_seg)
			? (
				(m_ofs == o.m_ofs)
				? (
					(m_len == o.m_len)
					? (
						(m_type == o.m_type)
						? (m_value < o.m_value)
						: (m_type < o.m_type)
					)
					: (m_len < o.m_len)
				)
				: (m_ofs < o.m_ofs)
			)
			: (m_seg < o.m_seg);
	}

private:
	unsigned int m_seg;
	unsigned int m_ofs;
	unsigned int m_len;
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
	BindLexem( unsigned int id_, const BindTerm& term)
		:BindTerm(term),m_id(id_){}

	unsigned int id() const		{return m_id;}

	bool operator < (const BindLexem& o) const
	{
		return BindTerm::operator <(o);
	}

private:
	unsigned int m_id;
};

}//namespace
#endif

