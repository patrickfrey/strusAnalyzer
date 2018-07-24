/*
 * Copyright (c) 2018 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Interface for the data built by a POS tagger
/// \file posTaggerDataInterface.hpp
#ifndef _STRUS_ANALYZER_POS_TAGGER_DATA_INTERFACE_HPP_INCLUDED
#define _STRUS_ANALYZER_POS_TAGGER_DATA_INTERFACE_HPP_INCLUDED
#include "strus/segmenterContextInterface.hpp"
#include <string>
#include <vector>

/// \brief strus toplevel namespace
namespace strus
{

/// \brief Forward declaration
class TokenMarkupContextInterface;

/// \brief Interface for the data built by a POS tagger
class PosTaggerDataInterface
{
public:
	/// \brief Output element of POS tagging
	class Element
	{
	public:
		const std::string& type() const {return m_type;}	///< Type of token (e.g. POS tag)
		const std::string& value() const {return m_value;}	///< Value of token (tagged value)

		/// \brief Constructor
		/// \param[in] type type of token (e.g. POS tag)
		/// \param[in] value value of token (tagged value)
		Element( const std::string& type_, const std::string& value_)
			:m_type(type_),m_value(value_){}
		/// \brief Copy constructor
		Element( const Element& o)
			:m_type(o.m_type),m_value(o.m_value){}

	private:
		std::string m_type;	///< Type of token (e.g. POS tag)
		std::string m_value;	///< Value of token (tagged value)
	};

	virtual ~PosTaggerDataInterface(){}

	/// \brief Define the tag to be used for a specific POS entity type
	/// \param[in] type POS entity type
	/// \param[in] tag to use for 'type'
	/// \remark Not allowed anymore after first insert
	virtual void defineTag( const std::string& type, const std::string& tag)=0;

	/// \brief Add a tagged text chunk
	/// \param[in] sequence tagged text chunk
	virtual void insert( int docno, const std::vector<Element>& sequence)=0;

	/// \brief Get a text chunk tagged
	/// \param[in] markupContext document context to do the markup of the POS tags
	/// \param[in] docno document number of the segment to tag
	/// \param[in,out] docitr iterator in the document (initial value 0)
	/// \param[in] segmentpos position of the segment in the original source
	/// \param[in] segmentptr pointer to content of segment to tag
	/// \param[in] segmentsize size of content of segment to tag in bytes
	virtual void markupSegment( TokenMarkupContextInterface* markupContext, int docno, int& docitr, const SegmenterPosition& segmentpos, const char* segmentptr, std::size_t segmentsize) const=0;
};

}//namespace
#endif

