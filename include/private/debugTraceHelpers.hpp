/*
 * Copyright (c) 2018 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Some utility helper functions for debug trace
/// \file debugTraceHelpers.hpp
#ifndef _STRUS_ANALYZER_DEBUG_TRACE_HELPERS_HPP_INCLUDED
#define _STRUS_ANALYZER_DEBUG_TRACE_HELPERS_HPP_INCLUDED
#include <string>

/// \brief strus toplevel namespace
namespace strus
{

std::string getStringContentStart( const std::string& content, int maxsize);

}//namespace
#endif

