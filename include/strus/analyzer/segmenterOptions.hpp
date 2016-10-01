/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Options to stear the segmenter behaviour or the structure of input if not self defined in the documents processed
/// \file segmenterInterface.hpp
#ifndef _STRUS_ANALYZER_SEGMENTER_OPTIONS_HPP_INCLUDED
#define _STRUS_ANALYZER_SEGMENTER_OPTIONS_HPP_INCLUDED
#include <vector>
#include <string>
#include <utility>

/// \brief strus toplevel namespace
namespace strus {
namespace analyzer {

/// \class SegmenterOptions
/// \brief Options to stear the segmenter behaviour or the structure of input if not self defined in the documents processed
/// \note Available options defined as key value pairs depend on the segmenter implementation
class SegmenterOptions
{
public:
	/// \brief Default constructor
	SegmenterOptions()
		:m_optar(0){}
	/// \brief Copy constructor
	SegmenterOptions( const SegmenterOptions& o)
		:m_optar(o.m_optar){}

	/// \brief One option item. Interpretation depends on the segmenter implementation
	typedef std::pair<std::string,std::string> Item;

	/// \brief Define a new option
	/// \param[in] name name of the option (case insensitive)
	/// \param[in] value value of the option
	/// \return this for cascading option definitions
	SegmenterOptions& operator()( const std::string& name, const std::string& value)
	{
		m_optar.push_back( Item( name, value));
		return *this;
	}

	/// \brief Get the list of all declared option items
	/// \return the option item list
	const std::vector<Item>& items() const
	{
		return m_optar;
	}

private:
	std::vector<Item> m_optar;
};

}}//namespace
#endif


