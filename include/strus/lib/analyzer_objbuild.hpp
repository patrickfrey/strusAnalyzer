/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Library with some convenient functions to create analyzer objects
/// \file "analyzer_objbuild.hpp"
#ifndef _STRUS_ANALYZER_OBJBUILD_LIB_HPP_INCLUDED
#define _STRUS_ANALYZER_OBJBUILD_LIB_HPP_INCLUDED

/// \brief strus toplevel namespace
namespace strus {

/// \brief Forward declaration
class AnalyzerObjectBuilderInterface;
/// \brief Forward declaration
class ErrorBufferInterface;

///\brief Create a storage object builder with the builders from the standard strus core libraries (without module support)
///\param[in] errorhnd error buffer interface
AnalyzerObjectBuilderInterface*
	createAnalyzerObjectBuilder_default(
		ErrorBufferInterface* errorhnd);

}//namespace
#endif

