/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Enumeration value to determine how an ordinal position of a term is assigned
/// \file term.hpp
#ifndef _STRUS_ANALYZER_POSITION_BIND_HPP_INCLUDED
#define _STRUS_ANALYZER_POSITION_BIND_HPP_INCLUDED

/// \brief strus toplevel namespace
namespace strus {
/// \brief analyzer parameter and return value objects namespace
namespace analyzer {

/// \enum PositionBind
/// \brief Determines how document ordinal positions are assigned to terms
enum PositionBind
{
	BindContent,		///< An element in the document that gets an own ordinal position assigned
	BindSuccessor,		///< An element in the document that gets the ordinal position of the element at the same position or the succeding content element assigned
	BindPredecessor,	///< An element in the document that gets the ordinal position of the element at the same position or the preceding content element assigned
	BindUnique		///< An element in the document that gets an own ordinal position assigned if it is not preceeded by another element with unique position assignment
};

}}//namespace
#endif

