/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Interface for the document segmenter
/// \file segmenterInterface.hpp
#ifndef _STRUS_ANALYZER_SEGMENTER_INTERFACE_HPP_INCLUDED
#define _STRUS_ANALYZER_SEGMENTER_INTERFACE_HPP_INCLUDED
#include <vector>
#include <string>
#include <utility>

/// \brief strus toplevel namespace
namespace strus
{
/// \brief Forward declaration
class SegmenterInstanceInterface;

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

	typedef std::pair<std::string,std::string> Item;

	SegmenterOptions& operator()( const std::string& name, const std::string& value)
	{
		m_optar.push_back( Item( name, value));
		return *this;
	}

	const std::vector<Item>& items() const
	{
		return m_optar;
	}
private:
	std::vector<Item> m_optar;
};


/// \class SegmenterInterface
/// \brief Defines an interface for creating instances of programs for document segmentation
class SegmenterInterface
{
public:
	/// \brief Destructor
	virtual ~SegmenterInterface(){}

	/// \brief Get the mime type accepted by this segmenter
	/// \return the mime type string
	virtual const char* mimeType() const=0;

	/// \brief Create a parameterizable segmenter instance
	/// \param[in] errorhnd analyzer error buffer interface for reporting exeptions and errors
	virtual SegmenterInstanceInterface* createInstance( const SegmenterOptions& opts=SegmenterOptions()) const=0;
};

}//namespace
#endif


