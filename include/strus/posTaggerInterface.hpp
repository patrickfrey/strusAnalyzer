/*
 * Copyright (c) 2018 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Interface for the construction of a POS tagger instance for a specified segmenter
/// \file posTaggerInterface.hpp
#ifndef _STRUS_ANALYZER_POS_TAGGER_INTERFACE_HPP_INCLUDED
#define _STRUS_ANALYZER_POS_TAGGER_INTERFACE_HPP_INCLUDED
#include "strus/analyzer/segmenterOptions.hpp"
#include <string>

/// \brief strus toplevel namespace
namespace strus
{

/// \brief Forward declaration
class SegmenterInterface;
/// \brief Forward declaration
class PosTaggerInstanceInterface;

/// \brief Interface for the construction of a POS tagger instance for a specified segmenter
class PosTaggerInterface
{
public:
	/// \brief Destructor
	virtual ~PosTaggerInterface(){}

	/// \brief Creates an instance for a specified segmenter
	/// \param[ín] segmenter segmenter instance to use
	/// \param[ín] opts options for the construction of the segmenter instance to use
	virtual PosTaggerInstanceInterface* createInstance(
			const SegmenterInterface* segmenter,
			const analyzer::SegmenterOptions& opts=analyzer::SegmenterOptions()) const=0;
};

}//namespace
#endif

