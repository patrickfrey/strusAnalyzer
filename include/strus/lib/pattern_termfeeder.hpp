/*
 * Copyright (c) 2016 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Library with the default implentation of the pattern term feeder interface
/// \file "analyzer_objbuild.hpp"
#ifndef _STRUS_ANALYZER_PATTERN_TERMFEEDER_LIB_HPP_INCLUDED
#define _STRUS_ANALYZER_PATTERN_TERMFEEDER_LIB_HPP_INCLUDED

/// \brief strus toplevel namespace
namespace strus {

/// \brief Forward declaration
class PatternTermFeederInterface;
/// \brief Forward declaration
class ErrorBufferInterface;

///\brief Create the term feeder interface for pattern matching on analyzer output as input
///\param[in] errorhnd error buffer interface
PatternTermFeederInterface*
	createPatternTermFeeder_default(
		ErrorBufferInterface* errorhnd);

}//namespace
#endif

