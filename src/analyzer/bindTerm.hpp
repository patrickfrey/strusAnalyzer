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
		:m_seg(o.m_seg),m_ofs(o.m_ofs),m_type(o.m_type),m_value(o.m_value),m_posbind(o.m_posbind){}
	BindTerm( unsigned int seg_, unsigned int ofs_, const std::string& type_, const std::string& value_, analyzer::PositionBind posbind_)
		:m_seg(seg_),m_ofs(ofs_),m_type(type_),m_value(value_),m_posbind(posbind_){}

	unsigned int seg() const			{return m_seg;}
	unsigned int ofs() const			{return m_ofs;}
	const std::string& type() const			{return m_type;}
	const std::string& value() const		{return m_value;}
	analyzer::PositionBind posbind() const		{return m_posbind;}

	bool operator < (const BindTerm& o) const
	{
		return (m_seg == o.m_seg)
			? (
				(m_ofs == o.m_ofs)
				? (
					(m_type == o.m_type)
					? (m_value < o.m_value)
					: (m_type < o.m_type)
				)
				: (m_ofs < o.m_ofs)
			)
			: (m_seg < o.m_seg);
	}
	
private:
	unsigned int m_seg;
	unsigned int m_ofs;
	std::string m_type;
	std::string m_value;
	analyzer::PositionBind m_posbind;
};

}//namespace
#endif

