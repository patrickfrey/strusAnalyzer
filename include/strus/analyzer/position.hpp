/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Structure describing a position in a document source by segment and offset
/// \note The segment is interpreted depending on the context it is used. It describes a chunk in the original
/// \file position.hpp
#ifndef _STRUS_ANALYZER_POSITION_HPP_INCLUDED
#define _STRUS_ANALYZER_POSITION_HPP_INCLUDED
#include "strus/base/stdint.h"


/// \brief strus toplevel namespace
namespace strus {
/// \brief analyzer parameter and return value objects namespace
namespace analyzer {

/// \brief Structure describing a position in a document source by segment and offset
struct Position
{
public:
	/// \brief Default constructor
	Position()
		:m_seg(0),m_ofs(0){}
	/// \brief Constructor
	/// \param[in] seg_ the position of the segment in the original source
	/// \param[in] ofs_ the byte position in the translated document segment (UTF-8)
	Position( int seg_, int ofs_)
		:m_seg(seg_),m_ofs(ofs_){}
	/// \brief Copy constructor
	Position( const Position& o)
		:m_seg(o.m_seg),m_ofs(o.m_ofs){}

	///\brief Get the position of the segment in the original source
	int seg() const	{return m_seg;}
	///\brief Get the byte position in the translated document segment (UTF-8)
	int ofs() const	{return m_ofs;}

	/// \brief Set the segment position
	void setSeg( int seg_)
	{
		m_seg = seg_;
	}
	/// \brief Set the byte offset in the segment
	void setOfs( int ofs_)
	{
		m_ofs = ofs_;
	}

	/// \brief Compare with another position
	int compare( const Position& o) const
	{
		return (m_seg == o.m_seg)
			? ((m_ofs < o.m_ofs) - (m_ofs > o.m_ofs))
			: ((m_seg < o.m_seg) - (m_seg > o.m_seg));
	}
	bool operator < (const Position& o) const
	{
		return (m_seg == o.m_seg)
			? (m_ofs < o.m_ofs)
			: (m_seg < o.m_seg);
	}
	bool operator > (const Position& o) const
	{
		return (m_seg == o.m_seg)
			? (m_ofs > o.m_ofs)
			: (m_seg > o.m_seg);
	}

private:
	uint32_t m_seg;		///< segment position in the document
	uint32_t m_ofs;		///< byte position in the document segment
};

}}//namespace
#endif

