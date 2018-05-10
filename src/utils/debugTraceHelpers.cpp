/*
 * Copyright (c) 2018 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Some utility helper functions for debug trace
/// \file debugTraceHelpers.hpp
#include "private/debugTraceHelpers.hpp"
#include "strus/base/utf8.hpp"

/// \brief strus toplevel namespace
using namespace strus;

std::string strus::getStringContentStart( const std::string& content, int maxsize)
{
	if (maxsize < 0 || maxsize > (int)content.size()) return content;
	std::string rt( content);
	while (maxsize > 0 && (rt[ maxsize-1] & B11000000) == B10000000) --maxsize;
	rt.resize( maxsize);
	return rt;
}

