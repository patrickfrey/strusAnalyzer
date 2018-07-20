/*
 * Copyright (c) 2018 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Implementation of the construction of a POS tagger instance for a specified segmenter
/// \file posTagger.hpp
#ifndef _STRUS_ANALYZER_POS_TAGGER_IMPLEMENTATION_HPP_INCLUDED
#define _STRUS_ANALYZER_POS_TAGGER_IMPLEMENTATION_HPP_INCLUDED
#include "strus/posTaggerInterface.hpp"
#include <string>

/// \brief strus toplevel namespace
namespace strus
{

/// \brief Forward declaration
class SegmenterInterface;
/// \brief Forward declaration
class PosTaggerInstanceInterface;

/// \brief Interface for the construction of a POS tagger instance for a specified segmenter
class PosTagger
	:public PosTaggerInterface
{
public:
	explicit PosTagger( ErrorBufferInterface* errorhnd_);
	virtual ~PosTagger();

	virtual PosTaggerInstanceInterface* createInstance(
			const SegmenterInterface* segmenter,
			const analyzer::SegmenterOptions& opts=analyzer::SegmenterOptions()) const;

private:
	ErrorBufferInterface* m_errorhnd;
};

}//namespace
#endif

