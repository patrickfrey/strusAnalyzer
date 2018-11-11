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
	/// \brief Output element declaration for POS tagging
	class Element
	{
	public:
		enum Type {Marker,Content,BoundToPrevious};
		static const char* typeName( Type t)
		{
			static const char* ar[] = {"Marker","Content","BoundToPrevious",0};
			return ar[ t];
		}
		const Type type() const {return m_type;}		///< Type of mapping
		const std::string& tag() const {return m_tag;}		///< Tag string
		const std::string& value() const {return m_value;}	///< Value of token (tagged value)

		/// \brief Constructor
		/// \param[in] type type of output mapping
		/// \param[in] tag name of tag
		/// \param[in] value value of token (tagged value)
		Element( const Type& type_, const std::string& tag_, const std::string& value_)
			:m_type(type_),m_tag(tag_),m_value(value_){}
		/// \brief Copy constructor
		Element( const Element& o)
			:m_type(o.m_type),m_tag(o.m_tag),m_value(o.m_value){}

	private:
		Type m_type;
		std::string m_tag;	///< Type of token (e.g. POS tag)
		std::string m_value;	///< Value of token (tagged value)
	};

	virtual ~PosTaggerDataInterface(){}

	/// \brief Declare a token to be ignored in the document elements, if it does not match
	/// \param[in] value value of the token
	/// \remark e.g. delimiter that might or might not be part of document segmentation because it belongs to the set of tokens potentially added in the input creation as delimiter.
	virtual void declareIgnoredToken( const std::string& value)=0;

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

